#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME 128
#define MAX_KEYS 3
#define MAX_CHILDREN 4
#define MAX_OPEN 32

typedef enum {
    TREE_ALPHA = 0,
    TREE_NUM   = 1,
    TREE_SYM   = 2
} TreeType;

typedef struct FileRecord {
    char name[MAX_NAME];
    int inode;
    int size;
} FileRecord;

typedef struct BPlusNode {
    int isLeaf;
    int numKeys;
    char keys[MAX_KEYS][MAX_NAME];

    struct BPlusNode *children[MAX_CHILDREN];
    FileRecord *records[MAX_KEYS];
    struct BPlusNode *next;
} BPlusNode;

typedef struct {
    int used;
    int fd;
    char name[MAX_NAME];
    char mode[4];
} OpenFile;

/* shared global state */
BPlusNode *treeRoots[3] = {NULL, NULL, NULL};
OpenFile openTable[MAX_OPEN] = {0};
int nextInode = 1;
int nextFD = 3;

/* ---------- classification ---------- */

TreeType classifyTree(const char *name)
{
    unsigned char c;

    if (name == NULL || name[0] == '\0')
        return TREE_SYM;

    c = (unsigned char)name[0];

    if (isalpha(c))
        return TREE_ALPHA;
    if (isdigit(c))
        return TREE_NUM;
    return TREE_SYM;
}

/* ---------- comparison helpers ---------- */

int ciCompare(const char *a, const char *b)
{
    while (*a && *b) {
        char ca = (char)tolower((unsigned char)*a);
        char cb = (char)tolower((unsigned char)*b);

        if (ca < cb) return -1;
        if (ca > cb) return 1;

        a++;
        b++;
    }

    if (*a == *b) return 0;
    return (*a == '\0') ? -1 : 1;
}

long leadingNumber(const char *s)
{
    long value = 0;
    int found = 0;

    while (*s && isdigit((unsigned char)*s)) {
        found = 1;
        value = value * 10 + (*s - '0');
        s++;
    }

    if (!found)
        return -1;

    return value;
}

unsigned int symbolHash(const char *s)
{
    unsigned int h = 2166136261u;

    while (*s) {
        h ^= (unsigned char)*s;
        h *= 16777619u;
        s++;
    }

    return h;
}

int compareByTree(TreeType type, const char *a, const char *b)
{
    if (type == TREE_ALPHA)
        return ciCompare(a, b);

    if (type == TREE_NUM) {
        long na = leadingNumber(a);
        long nb = leadingNumber(b);

        if (na < nb) return -1;
        if (na > nb) return 1;
        return strcmp(a, b);
    }

    {
        unsigned int ha = symbolHash(a);
        unsigned int hb = symbolHash(b);

        if (ha < hb) return -1;
        if (ha > hb) return 1;
        return strcmp(a, b);
    }
}

/* ---------- creation helpers ---------- */

BPlusNode *makeNode(int isLeaf)
{
    BPlusNode *node = (BPlusNode *)malloc(sizeof(BPlusNode));
    int i;

    if (node == NULL)
        return NULL;

    node->isLeaf = isLeaf;
    node->numKeys = 0;
    node->next = NULL;

    for (i = 0; i < MAX_CHILDREN; i++)
        node->children[i] = NULL;

    for (i = 0; i < MAX_KEYS; i++)
        node->records[i] = NULL;

    return node;
}

FileRecord *makeRecord(const char *name)
{
    FileRecord *rec = (FileRecord *)malloc(sizeof(FileRecord));

    if (rec == NULL)
        return NULL;

    strncpy(rec->name, name, MAX_NAME - 1);
    rec->name[MAX_NAME - 1] = '\0';
    rec->inode = nextInode++;
    rec->size = 0;

    return rec;
}

/* ---------- search ---------- */

BPlusNode *findLeaf(BPlusNode *root, TreeType type, const char *name)
{
    int i;

    if (root == NULL)
        return NULL;

    while (!root->isLeaf) {
        i = 0;
        while (i < root->numKeys &&
               compareByTree(type, name, root->keys[i]) >= 0) {
            i++;
        }
        root = root->children[i];
    }

    return root;
}

FileRecord *bptSearch(BPlusNode *root, TreeType type, const char *name)
{
    BPlusNode *leaf;
    int i;

    leaf = findLeaf(root, type, name);
    if (leaf == NULL)
        return NULL;

    for (i = 0; i < leaf->numKeys; i++) {
        if (compareByTree(type, name, leaf->keys[i]) == 0)
            return leaf->records[i];
    }

    return NULL;
}

FileRecord *searchCentral(const char *name, TreeType *outType)
{
    TreeType type;

    if (name == NULL || name[0] == '\0')
        return NULL;

    type = classifyTree(name);

    if (outType != NULL)
        *outType = type;

    return bptSearch(treeRoots[type], type, name);
}

/* ---------- insertion helpers ---------- */

void insertIntoLeaf(BPlusNode *leaf, TreeType type, FileRecord *rec)
{
    int i;
    int pos = 0;

    while (pos < leaf->numKeys &&
           compareByTree(type, rec->name, leaf->keys[pos]) > 0) {
        pos++;
    }

    for (i = leaf->numKeys; i > pos; i--) {
        strcpy(leaf->keys[i], leaf->keys[i - 1]);
        leaf->records[i] = leaf->records[i - 1];
    }

    strcpy(leaf->keys[pos], rec->name);
    leaf->records[pos] = rec;
    leaf->numKeys++;
}

BPlusNode *findParent(BPlusNode *root, BPlusNode *child)
{
    int i;
    BPlusNode *result;

    if (root == NULL || root->isLeaf)
        return NULL;

    for (i = 0; i <= root->numKeys; i++) {
        if (root->children[i] == child)
            return root;
    }

    for (i = 0; i <= root->numKeys; i++) {
        result = findParent(root->children[i], child);
        if (result != NULL)
            return result;
    }

    return NULL;
}

void insertIntoInternal(BPlusNode *node, TreeType type,
                        const char *key, BPlusNode *rightChild)
{
    int i;
    int pos = 0;

    while (pos < node->numKeys &&
           compareByTree(type, key, node->keys[pos]) > 0) {
        pos++;
    }

    for (i = node->numKeys; i > pos; i--)
        strcpy(node->keys[i], node->keys[i - 1]);

    for (i = node->numKeys + 1; i > pos + 1; i--)
        node->children[i] = node->children[i - 1];

    strcpy(node->keys[pos], key);
    node->children[pos + 1] = rightChild;
    node->numKeys++;
}

BPlusNode *insertIntoParent(BPlusNode *root, TreeType type,
                            BPlusNode *left, const char *key, BPlusNode *right);

BPlusNode *splitInternalAndInsert(BPlusNode *root, TreeType type,
                                  BPlusNode *node, const char *key,
                                  BPlusNode *rightChild)
{
    char tempKeys[MAX_KEYS + 1][MAX_NAME];
    BPlusNode *tempChildren[MAX_CHILDREN + 1];
    int i, j, pos = 0;
    int split;
    BPlusNode *newInternal;
    char promoteKey[MAX_NAME];

    while (pos < node->numKeys &&
           compareByTree(type, key, node->keys[pos]) > 0) {
        pos++;
    }

    for (i = 0, j = 0; i < node->numKeys; i++, j++) {
        if (j == pos) j++;
        strcpy(tempKeys[j], node->keys[i]);
    }

    for (i = 0, j = 0; i <= node->numKeys; i++, j++) {
        if (j == pos + 1) j++;
        tempChildren[j] = node->children[i];
    }

    strcpy(tempKeys[pos], key);
    tempChildren[pos + 1] = rightChild;

    split = (MAX_KEYS + 1) / 2;
    strcpy(promoteKey, tempKeys[split]);

    node->numKeys = 0;
    for (i = 0; i < split; i++) {
        strcpy(node->keys[i], tempKeys[i]);
        node->children[i] = tempChildren[i];
        node->numKeys++;
    }
    node->children[i] = tempChildren[i];

    newInternal = makeNode(0);
    if (newInternal == NULL)
        return root;

    for (i = split + 1, j = 0; i < MAX_KEYS + 1; i++, j++) {
        strcpy(newInternal->keys[j], tempKeys[i]);
        newInternal->children[j] = tempChildren[i];
        newInternal->numKeys++;
    }
    newInternal->children[j] = tempChildren[i];

    return insertIntoParent(root, type, node, promoteKey, newInternal);
}

BPlusNode *insertIntoParent(BPlusNode *root, TreeType type,
                            BPlusNode *left, const char *key, BPlusNode *right)
{
    BPlusNode *parent;
    BPlusNode *newRoot;

    if (root == left) {
        newRoot = makeNode(0);
        if (newRoot == NULL)
            return root;

        strcpy(newRoot->keys[0], key);
        newRoot->children[0] = left;
        newRoot->children[1] = right;
        newRoot->numKeys = 1;
        return newRoot;
    }

    parent = findParent(root, left);
    if (parent == NULL)
        return root;

    if (parent->numKeys < MAX_KEYS) {
        insertIntoInternal(parent, type, key, right);
        return root;
    }

    return splitInternalAndInsert(root, type, parent, key, right);
}

BPlusNode *splitLeafAndInsert(BPlusNode *root, TreeType type,
                              BPlusNode *leaf, FileRecord *rec)
{
    char tempKeys[MAX_KEYS + 1][MAX_NAME];
    FileRecord *tempRecords[MAX_KEYS + 1];
    int i, j, pos = 0;
    int split;
    BPlusNode *newLeaf;
    char promoteKey[MAX_NAME];

    while (pos < leaf->numKeys &&
           compareByTree(type, rec->name, leaf->keys[pos]) > 0) {
        pos++;
    }

    for (i = 0, j = 0; i < leaf->numKeys; i++, j++) {
        if (j == pos) j++;
        strcpy(tempKeys[j], leaf->keys[i]);
        tempRecords[j] = leaf->records[i];
    }

    strcpy(tempKeys[pos], rec->name);
    tempRecords[pos] = rec;

    split = (MAX_KEYS + 1) / 2;
    leaf->numKeys = 0;

    for (i = 0; i < split; i++) {
        strcpy(leaf->keys[i], tempKeys[i]);
        leaf->records[i] = tempRecords[i];
        leaf->numKeys++;
    }

    newLeaf = makeNode(1);
    if (newLeaf == NULL)
        return root;

    for (i = split, j = 0; i < MAX_KEYS + 1; i++, j++) {
        strcpy(newLeaf->keys[j], tempKeys[i]);
        newLeaf->records[j] = tempRecords[i];
        newLeaf->numKeys++;
    }

    newLeaf->next = leaf->next;
    leaf->next = newLeaf;

    strcpy(promoteKey, newLeaf->keys[0]);

    return insertIntoParent(root, type, leaf, promoteKey, newLeaf);
}

BPlusNode *bptInsert(BPlusNode *root, TreeType type, const char *name, int *inserted)
{
    BPlusNode *leaf;
    FileRecord *rec;

    if (root == NULL) {
        root = makeNode(1);
        if (root == NULL) {
            *inserted = 0;
            return NULL;
        }

        rec = makeRecord(name);
        if (rec == NULL) {
            *inserted = 0;
            return root;
        }

        strcpy(root->keys[0], rec->name);
        root->records[0] = rec;
        root->numKeys = 1;
        *inserted = 1;
        return root;
    }

    if (bptSearch(root, type, name) != NULL) {
        *inserted = 0;
        return root;
    }

    leaf = findLeaf(root, type, name);
    rec = makeRecord(name);
    if (rec == NULL) {
        *inserted = 0;
        return root;
    }

    if (leaf->numKeys < MAX_KEYS) {
        insertIntoLeaf(leaf, type, rec);
        *inserted = 1;
        return root;
    }

    *inserted = 1;
    return splitLeafAndInsert(root, type, leaf, rec);
}

int insertFileCentral(const char *name)
{
    TreeType type;
    int inserted = 0;

    if (name == NULL || name[0] == '\0')
        return 0;

    type = classifyTree(name);
    treeRoots[type] = bptInsert(treeRoots[type], type, name, &inserted);

    return inserted;
}
