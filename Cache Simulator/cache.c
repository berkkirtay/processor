#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Please compile code as in the following "gcc cache.c -lm"

int L1s = 0, L1E = 2, L1b = 3, L2s = 1, L2E = 2, L2b = 3;
char traceLines[10000][64];

unsigned char *RAMBuffer = NULL;
int RAMSize = 0;

// A timer for cache operations.
int timer = 0;

// Cache structures
struct line
{
    int tag;
    int isValid;
    int time;
    unsigned char *block;
} typedef line;

struct cacheSet
{
    line *lines; // E-way Set Associative
} typedef cacheSet;

struct genecericCache
{
    int hits;
    int misses;
    int evictions;
    cacheSet *cache;
} typedef genecericCache;

genecericCache *L1I = NULL;
genecericCache *L1D = NULL;
genecericCache *L2 = NULL;

// Prototypes

void input(char *testName);
void setCaches();
void cache();
void execute(char operation, char *address, int size, char *data);
void convertToBinary(char *address, int decimalAddress, int *retAddr);
void printCaches();
void freeCaches();

int main(int argc, char *argv[])
{
    if (argc != 15)
    {
        printf("Wrong command line arguments!\n");
        return 0;
    }
    //-L1s 0 -L1E 2 -L1b 3 -L2s 1 -L2E 2 -L2b 3 -t test1.trace

    L1s = atoi(argv[2]);
    L1E = atoi(argv[4]);
    L1b = atoi(argv[6]);
    L2s = atoi(argv[8]);
    L2E = atoi(argv[10]);
    L2b = atoi(argv[12]);

    if (!(L1s >= 0 && L1E >= 1 && L1b >= 0 && L2s >= 0 && L2E >= 1 && L2b >= 0))
    {
        printf("Input parameters error!");
        exit(EXIT_FAILURE);
    }

    char testName[strlen(argv[14]) + 1];
    strncpy(testName, argv[14], strlen(argv[14]) + 1);

    input(testName);
    setCaches();
    cache();
    printCaches();
    freeCaches();
}

void input(char *testName)
{
    // Trace file input
    FILE *file = fopen(testName, "r");
    if (file == NULL)
    {
        perror("Error: ");
        exit(EXIT_FAILURE);
    }

    int index = 0;
    while (fgets(traceLines[index], sizeof(traceLines[index]), file) != NULL)
    {
        index++;
    }
    fclose(file);

    // RAM input
    FILE *file2 = fopen("RAM.dat", "rb");
    if (file2 == NULL)
    {
        perror("Error: ");
        exit(EXIT_FAILURE);
    }

    fseek(file2, 0, SEEK_END);
    RAMSize = ftell(file2);
    fseek(file2, 0, SEEK_SET);

    RAMBuffer = malloc(sizeof(char) * (RAMSize + 1));
    fread(RAMBuffer, sizeof *RAMBuffer, RAMSize, file2);
    RAMBuffer[RAMSize] = '\0';
    fclose(file2);
}

void setCaches()
{
    // L1I, L1D
    int S = pow(2, L1s);
    cacheSet *sets = malloc(sizeof(cacheSet) * S);
    cacheSet *sets2 = malloc(sizeof(cacheSet) * S);
    int E = L1E;
    int B = pow(2, L1b);

    int i;
    for (i = 0; i < S; i++)
    {
        sets[i].lines = malloc(sizeof(line) * E);
        sets2[i].lines = malloc(sizeof(line) * E);

        int j;
        for (j = 0; j < E; j++)
        {
            sets[i].lines[j].isValid = 0;
            sets[i].lines[j].tag = -1;
            sets[i].lines[j].block = malloc(sizeof *sets[i].lines[j].block * B);
            sets2[i].lines[j].isValid = 0;
            sets2[i].lines[j].tag = -1;
            sets2[i].lines[j].block = malloc(sizeof *sets2[i].lines[j].block * B);
        }
    }

    L1I = malloc(sizeof(genecericCache) + sizeof(sets));
    L1D = malloc(sizeof(genecericCache) + sizeof(sets));
    L1I->cache = sets;
    L1D->cache = sets2;

    L1I->misses = 0;
    L1I->hits = 0;
    L1I->evictions = 0;

    L1D->misses = 0;
    L1D->hits = 0;
    L1D->evictions = 0;

    //L2
    S = pow(2, L2s);
    cacheSet *sets3 = malloc(sizeof(cacheSet) * S);
    E = L2E;
    B = pow(2, L2b);

    for (i = 0; i < S; i++)
    {
        sets3[i].lines = malloc(sizeof(line) * E);

        int j;
        for (j = 0; j < E; j++)
        {
            sets3[i].lines[j].isValid = 0;
            sets3[i].lines[j].tag = -1;
            sets3[i].lines[j].block = malloc(sizeof *sets3[i].lines[j].block * B);
        }
    }

    L2 = malloc(sizeof(genecericCache) + sizeof(sets));
    L2->cache = sets3;

    L2->misses = 0;
    L2->hits = 0;
    L2->evictions = 0;
}

void cache()
{
    int index = 0;
    while (traceLines[index][0] != '\0')
    {
        char operation;
        char *address = NULL;
        int size = -1;
        char *data = NULL;

        char tempLine[64];
        strncpy(tempLine, traceLines[index], 64);

        char *token = strtok(tempLine, " ");
        operation = token[0];
        token = strtok(NULL, ", \n");

        while (token != NULL)
        {
            if (address == NULL)
            {
                address = malloc(strlen(token) + 1);
                strncpy(address, token, strlen(token));
                address[strlen(token)] = '\0';
            }
            else if (size == -1)
            {
                size = atoi(token);
            }
            else if (data == NULL)
            {
                data = malloc(strlen(token) + 1);
                strncpy(data, token, strlen(token));
                data[strlen(token)] = '\0';
            }

            token = strtok(NULL, ", \n");
        }

        execute(operation, address, size, data);
        free(address);
        free(data);
        index++;
    }

    printf("\nL1I-hits:%d L1I-misses:%d L1I-evictions:%d\n", L1I->hits, L1I->misses, L1I->evictions);
    printf("L1D-hits:%d L1D-misses:%d L1D-evictions:%d\n", L1D->hits, L1D->misses, L1D->evictions);
    printf("L2-hits:%d L2-misses:%d L2-evictions:%d\n", L2->hits, L2->misses, L2->evictions);
}

void execute(char operation, char *address, int size, char *data)
{
    int addressLen = 8 * 4;
    int decimalAddress = 0;
    int index = 0;
    int addLen = strlen(address);

    while (addLen - 1 - index >= 0)
    {
        int temp;
        if (address[addLen - 1 - index] - 'a' >= 0)
        {
            temp = address[addLen - 1 - index] - 'a' + 10;
        }
        else
        {
            temp = address[addLen - 1 - index] - '0';
        }
        decimalAddress += temp * pow(16, index);
        index++;
    }

    int binaryAddress[addressLen];
    convertToBinary(address, decimalAddress, binaryAddress);

    int B;
    if (L1b > L2b)
    {
        B = pow(2, L1b);
    }
    else
    {
        B = pow(2, L2b);
    }

    unsigned char newData[B + 1];
    index = 0;

    strncpy(newData, &RAMBuffer[0] + decimalAddress - (decimalAddress % 8), B);
    newData[B] = '\0';

    char L1DStatus[5];
    char L1IStatus[5];
    char L2Status[5];

    switch (operation)
    {
    // Instruction Load
    case 'I':
    {
        printf("I %s, %d\n", address, size);

        //L1I I Load
        int L1IsetNumber = 0;
        index = addressLen - L1b - 1;
        int i = 0;
        while (i < L1s)
        {
            L1IsetNumber += binaryAddress[index--] * pow(2, i++);
        }

        int L1Itag = 0;
        index = addressLen - L1b - L1s - 1;
        i = 0;
        while (index >= 0)
        {
            L1Itag += binaryAddress[index--] * pow(2, i++);
        }
        int L1IhitFlag = 0;
        for (i = 0; i < L1E; i++)
        {
            if (L1I->cache[L1IsetNumber].lines[i].tag == L1Itag)
            {
                L1IhitFlag = 1;
                break;
            }
        }

        int L1IEvictionFlag = 0;
        if (L1IhitFlag == 1)
        {
            L1I->hits++;
        }
        else
        {
            L1I->misses++;
            for (i = 0; i < L1E; i++)
            {
                if (L1I->cache[L1IsetNumber].lines[i].isValid == 1)
                {
                    L1IEvictionFlag = 1;
                }
                else
                {
                    L1IEvictionFlag = 0;
                    break;
                }
            }

            // Put the data to the free set.
            if (L1IEvictionFlag == 0)
            {
                L1I->cache[L1IsetNumber].lines[i].isValid = 1;
                L1I->cache[L1IsetNumber].lines[i].tag = L1Itag;
                L1I->cache[L1IsetNumber].lines[i].time = timer++;
                strncpy(L1I->cache[L1IsetNumber].lines[i].block, newData, pow(2, L1b) + 1);
            }
            // Eviction case
            else
            {
                int minTime = 100000;
                for (i = 0; i < L1E; i++)
                {
                    if (L1I->cache[L1IsetNumber].lines[i].time < minTime)
                    {
                        minTime = L1I->cache[L1IsetNumber].lines[i].time;
                        index = i;
                    }
                }
                L1I->cache[L1IsetNumber].lines[index].tag = L1Itag;
                L1I->cache[L1IsetNumber].lines[index].time = timer++;
                strncpy(L1I->cache[L1IsetNumber].lines[index].block, newData, pow(2, L1b) + 1);
                L1I->evictions++;
            }
        }

        //L2 I Load
        int L2setNumber = 0;
        index = addressLen - L2b - 1;
        i = 0;
        while (i < L2s)
        {
            L2setNumber += binaryAddress[index--] * pow(2, i++);
        }

        int L2tag = 0;
        index = addressLen - L2b - L2s - 1;
        i = 0;
        while (index >= 0)
        {
            L2tag += binaryAddress[index--] * pow(2, i++);
        }

        int L2hitFlag = 0;
        for (i = 0; i < L2E; i++)
        {
            if (L2->cache[L2setNumber].lines[i].tag == L2tag)
            {
                L2hitFlag = 1;
                break;
            }
        }

        int L2EvictionFlag = 0;
        if (L2hitFlag == 1)
        {
            L2->hits++;
        }
        else
        {
            L2->misses++;
            for (i = 0; i < L2E; i++)
            {
                if (L2->cache[L2setNumber].lines[i].isValid == 1)
                {
                    L2EvictionFlag = 1;
                }
                else
                {
                    L2EvictionFlag = 0;
                    break;
                }
            }
            if (L2EvictionFlag == 0)
            {
                L2->cache[L2setNumber].lines[i].isValid = 1;
                L2->cache[L2setNumber].lines[i].tag = L2tag;
                L2->cache[L2setNumber].lines[i].time = timer++;
                strncpy(L2->cache[L2setNumber].lines[i].block, newData, pow(2, L2b) + 1);
            }

            // Eviction case
            else
            {
                int minTime = 100000;
                for (i = 0; i < L2E; i++)
                {
                    if (L2->cache[L2setNumber].lines[i].time < minTime)
                    {
                        minTime = L2->cache[L2setNumber].lines[i].time;
                        index = i;
                    }
                }
                L2->cache[L2setNumber].lines[index].tag = L2tag;
                L2->cache[L2setNumber].lines[index].time = timer++;
                strncpy(L2->cache[L2setNumber].lines[index].block, newData, pow(2, L2b) + 1);
                L2->evictions++;
            }
        }

        if (L1IhitFlag == 0)
            strcpy(L1IStatus, "miss");
        else
            strcpy(L1IStatus, "hit");
        if (L2hitFlag == 0)
            strcpy(L2Status, "miss");
        else
            strcpy(L2Status, "hit");

        printf("   L1I %s, L2 %s\n", L1IStatus, L2Status);
        printf("   Place in L2 set %d, L1I %d\n", L2setNumber, L1IsetNumber);

        break;
    }

    // Data Load
    case 'L':
    {
        printf("L %s, %d\n", address, size);

        //L1D Load
        int L1DsetNumber = 0;
        index = addressLen - L1b - 1;
        int i = 0;
        while (i < L1s)
        {
            L1DsetNumber += binaryAddress[index--] * pow(2, i++);
        }

        int L1Dtag = 0;
        index = addressLen - L1b - L1s - 1;
        i = 0;
        while (index >= 0)
        {
            L1Dtag += binaryAddress[index--] * pow(2, i++);
        }
        int L1DhitFlag = 0;
        for (i = 0; i < L1E; i++)
        {
            if (L1I->cache[L1DsetNumber].lines[i].tag == L1Dtag)
            {
                L1DhitFlag = 1;
                break;
            }
        }

        int L1DEvictionFlag = 0;
        if (L1DhitFlag == 1)
        {
            L1D->hits++;
        }
        else
        {
            L1D->misses++;
            for (i = 0; i < L1E; i++)
            {
                if (L1D->cache[L1DsetNumber].lines[i].isValid == 1)
                {
                    L1DEvictionFlag = 1;
                }
                else
                {
                    L1DEvictionFlag = 0;
                    break;
                }
            }
            // Put the data to the free set.
            if (L1DEvictionFlag == 0)
            {
                L1D->cache[L1DsetNumber].lines[i].isValid = 1;
                L1D->cache[L1DsetNumber].lines[i].tag = L1Dtag;
                L1D->cache[L1DsetNumber].lines[i].time = timer++;
                strncpy(L1D->cache[L1DsetNumber].lines[i].block, newData, pow(2, L1b) + 1);
            }
            // Eviction case
            else
            {
                int minTime = 100000;
                for (i = 0; i < L1E; i++)
                {
                    if (L1D->cache[L1DsetNumber].lines[i].time < minTime)
                    {
                        minTime = L1D->cache[L1DsetNumber].lines[i].time;
                        index = i;
                    }
                }
                L1D->cache[L1DsetNumber].lines[index].tag = L1Dtag;
                L1D->cache[L1DsetNumber].lines[index].time = timer++;
                strncpy(L1D->cache[L1DsetNumber].lines[index].block, newData, pow(2, L1b) + 1);
                L1D->evictions++;
            }
        }

        //L2 I Load
        int L2setNumber = 0;
        index = addressLen - L2b - 1;
        i = 0;
        while (i < L2s)
        {
            L2setNumber += binaryAddress[index--] * pow(2, i++);
        }

        int L2tag = 0;
        index = addressLen - L2b - L2s - 1;
        i = 0;
        while (index >= 0)
        {
            L2tag += binaryAddress[index--] * pow(2, i++);
        }

        int L2hitFlag = 0;
        for (i = 0; i < L2E; i++)
        {
            if (L2->cache[L2setNumber].lines[i].tag == L2tag)
            {
                L2hitFlag = 1;
                break;
            }
        }

        int L2EvictionFlag = 0;
        if (L2hitFlag == 1)
        {
            L2->hits++;
        }

        else
        {
            L2->misses++;
            for (i = 0; i < L2E; i++)
            {
                if (L2->cache[L2setNumber].lines[i].isValid == 1)
                {
                    L2EvictionFlag = 1;
                }
                else
                {
                    L2EvictionFlag = 0;
                    break;
                }
            }
            if (L2EvictionFlag == 0)
            {
                L2->cache[L2setNumber].lines[i].isValid = 1;
                L2->cache[L2setNumber].lines[i].tag = L2tag;
                L2->cache[L2setNumber].lines[i].time = timer++;
                strncpy(L2->cache[L2setNumber].lines[i].block, newData, pow(2, L2b) + 1);
            }
            // Eviction case
            else
            {
                int minTime = 100000;
                for (i = 0; i < L2E; i++)
                {
                    if (L2->cache[L2setNumber].lines[i].time < minTime)
                    {
                        minTime = L2->cache[L2setNumber].lines[i].time;
                        index = i;
                    }
                }
                L2->cache[L2setNumber].lines[index].tag = L2tag;
                L2->cache[L2setNumber].lines[index].time = timer++;
                strncpy(L2->cache[L2setNumber].lines[index].block, newData, pow(2, L2b) + 1);
                L2->evictions++;
            }
        }

        if (L1DhitFlag == 0)
            strcpy(L1DStatus, "miss");
        else
            strcpy(L1DStatus, "hit");
        if (L2hitFlag == 0)
            strcpy(L2Status, "miss");
        else
            strcpy(L2Status, "hit");

        printf("   L1D %s, L2 %s\n", L1DStatus, L2Status);
        printf("   Place in L2 set %d, L1D %d\n", L2setNumber, L1DsetNumber);
        break;
    }
    // Data Store
    case 'S':
    {
        printf("S %s, %d, %s\n", address, size, data);

        //L1D Store
        int L1DsetNumber = 0;
        index = addressLen - L1b - 1;
        int i = 0;
        while (i < L1s)
        {
            L1DsetNumber += binaryAddress[index--] * pow(2, i++);
        }

        int L1Dtag = 0;
        index = addressLen - L1b - L1s - 1;
        i = 0;
        while (index >= 0)
        {
            L1Dtag += binaryAddress[index--] * pow(2, i++);
        }

        int L1DhitFlag = 0;
        for (i = 0; i < L1E; i++)
        {
            if (L1D->cache[L1DsetNumber].lines[i].tag == L1Dtag)
            {
                L1DhitFlag = 1;
                index = i;
                break;
            }
        }

        // write-through and no write allocate
        if (L1DhitFlag == 1)
        {
            L1D->hits++;
            int j = decimalAddress % 10;
            index = decimalAddress;
            unsigned long hexData = strtoul(data, NULL, 16);
            memcpy(&RAMBuffer[0] + index, &hexData, size);
            memcpy(&L1D->cache[L1DsetNumber].lines[i].block[0] + j, &hexData, size);
        }
        else
        {
            L1D->misses++;
            int j;
            index = decimalAddress;
            unsigned long hexData = strtoul(data, NULL, 16);
            memcpy(&RAMBuffer[0] + index, &hexData, size);
        }

        //L2 Store
        int L2setNumber = 0;
        index = addressLen - L2b - 1;
        i = 0;
        while (i < L2s)
        {
            L2setNumber += binaryAddress[index--] * pow(2, i++);
        }

        int L2tag = 0;
        index = addressLen - L2b - L2s - 1;
        i = 0;
        while (index >= 0)
        {
            L2tag += binaryAddress[index--] * pow(2, i++);
        }

        int L2hitFlag = 0;
        for (i = 0; i < L2E; i++)
        {
            if (L2->cache[L2setNumber].lines[i].tag == L2tag)
            {
                L2hitFlag = 1;
                index = i;
                break;
            }
        }
        if (L2hitFlag == 1)
        {
            L2->hits++;
            int j = decimalAddress % 10;
            index = decimalAddress;
            unsigned long hexData = strtoul(data, NULL, 16);
            memcpy(&RAMBuffer[0] + index, &hexData, size);
            memcpy(&L2->cache[L2setNumber].lines[i].block[0] + j, &hexData, size);
        }
        else
        {
            L2->misses++;
            int j;
            index = decimalAddress;
            unsigned long hexData = strtoul(data, NULL, 16);
            memcpy(&RAMBuffer[0] + index, &hexData, size);
        }

        if (L1DhitFlag == 0)
            strcpy(L1DStatus, "miss");
        else
            strcpy(L1DStatus, "hit");
        if (L2hitFlag == 0)
            strcpy(L2Status, "miss");
        else
            strcpy(L2Status, "hit");

        printf("   L1D %s, L2 %s\n", L1DStatus, L2Status);
        if (L1DhitFlag == 0 || L2hitFlag == 0)
        {
            printf("   Store in RAM\n");
        }
        else
        {
            printf("   Store in L1D, L2, RAM\n");
        }

        break;
    }

    // Data Modify
    case 'M':
        execute('L', address, size, data);
        execute('S', address, size, data);
        break;
    }
}

void printCaches()
{
    FILE *cacheL1I = fopen("cache-L1I.txt", "w");

    fprintf(cacheL1I, "L1I Cache Contents\n");
    int S = pow(2, L1s);
    int B = pow(2, L1b);
    int i;
    int j;
    for (i = 0; i < S; i++)
    {
        fprintf(cacheL1I, "%d. set\n", i);
        fprintf(cacheL1I, "---------------------\n");
        for (j = 0; j < L1E; j++)
        {
            int tag = L1I->cache[i].lines[j].tag;
            if (tag == -1)
            {
                fprintf(cacheL1I, "---Empty block---\n");
            }
            else
            {
                fprintf(cacheL1I, "0x%02X | ", tag);
                int x;
                fprintf(cacheL1I, "0x");
                for (x = 0; x < B; x++)
                {
                    fprintf(cacheL1I, "%02X", L1I->cache[i].lines[j].block[x]);
                }
                fprintf(cacheL1I, "\n");
            }
        }
        fprintf(cacheL1I, "---------------------\n");
    }

    fclose(cacheL1I);

    FILE *cacheL1D = fopen("cache-L1D.txt", "w");

    fprintf(cacheL1D, "L1D Cache Contents\n");
    for (i = 0; i < S; i++)
    {
        fprintf(cacheL1D, "%d. set\n", i);
        fprintf(cacheL1D, "---------------------\n");
        for (j = 0; j < L1E; j++)
        {
            int tag = L1D->cache[i].lines[j].tag;
            if (tag == -1)
            {
                fprintf(cacheL1D, "---Empty block---\n");
            }
            else
            {
                fprintf(cacheL1D, "0x%02X | ", tag);
                int x;
                fprintf(cacheL1D, "0x");
                for (x = 0; x < B; x++)
                {
                    fprintf(cacheL1D, "%02X", L1D->cache[i].lines[j].block[x]);
                }
                fprintf(cacheL1D, "\n");
            }
        }
        fprintf(cacheL1D, "---------------------\n");
    }

    fclose(cacheL1D);

    FILE *cacheL2 = fopen("cache-L2.txt", "w");

    S = pow(2, L2s);
    B = pow(2, L2b);

    fprintf(cacheL2, "L2 Cache Contents\n");
    for (i = 0; i < S; i++)
    {
        fprintf(cacheL2, "%d. set\n", i);
        fprintf(cacheL2, "---------------------\n");
        for (j = 0; j < L2E; j++)
        {
            int tag = L2->cache[i].lines[j].tag;
            if (tag == -1)
            {
                fprintf(cacheL2, "---Empty block---\n");
            }
            else
            {
                fprintf(cacheL2, "0x%02X | ", tag);
                int x;
                fprintf(cacheL2, "0x");
                for (x = 0; x < B; x++)
                {
                    fprintf(cacheL2, "%02X", L2->cache[i].lines[j].block[x]);
                }
                fprintf(cacheL2, "\n");
            }
        }
        fprintf(cacheL2, "---------------------\n");
    }
    fclose(cacheL2);

    // Writing out the new RAM image.
    FILE *RAMFile = fopen("newRAM.dat", "wb");
    fwrite(RAMBuffer, 1, RAMSize, RAMFile);
    fclose(RAMFile);

    printf("Output files are written!\n");
}

void convertToBinary(char *address, int decimalAddress, int *retAddr)
{
    int addrLen = 8 * 4;
    int binaryAddress[addrLen];
    int tempArr[addrLen];
    int index = 0;
    while (index < addrLen)
    {
        binaryAddress[index++] = 0;
    }
    index = 0;
    while (decimalAddress > 0)
    {
        binaryAddress[addrLen - 1 - index] = decimalAddress % 2;
        decimalAddress /= 2;
        index++;
    }
    int i;
    for (i = 0; i < addrLen; i++)
    {
        retAddr[i] = binaryAddress[i];
    }
}

void freeCaches()
{
    free(RAMBuffer);
    free(L1I);
    free(L1D);
    free(L2);
}