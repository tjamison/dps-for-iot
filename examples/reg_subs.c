#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <dps/dbg.h>
#include <dps/dps.h>
#include <dps/synchronous.h>
#include <dps/registration.h>
#include <dps/event.h>

static int quiet = DPS_FALSE;

static uint8_t AckMsg[] = "This is an ACK";

#define NUM_KEYS 2

static DPS_UUID keyId[NUM_KEYS] = { 
    { .val = { 0xed,0x54,0x14,0xa8,0x5c,0x4d,0x4d,0x15,0xb6,0x9f,0x0e,0x99,0x8a,0xb1,0x71,0xf2 } },
    { .val = { 0x53,0x4d,0x2a,0x4b,0x98,0x76,0x1f,0x25,0x6b,0x78,0x3c,0xc2,0xf8,0x12,0x90,0xcc } }
};

/*
 * Preshared keys for testing only - DO NOT USE THESE KEYS IN A REAL APPLICATION!!!!
 */
static uint8_t keyData[NUM_KEYS][16] = {
    { 0x77,0x58,0x22,0xfc,0x3d,0xef,0x48,0x88,0x91,0x25,0x78,0xd0,0xe2,0x74,0x5c,0x10 },
    { 0x39,0x12,0x3e,0x7f,0x21,0xbc,0xa3,0x26,0x4e,0x6f,0x3a,0x21,0xa4,0xf1,0xb5,0x98 }
};

DPS_Status GetKey(DPS_Node* node, const DPS_UUID* kid, uint8_t* key, size_t keyLen)
{
    size_t i;

    for (i = 0; i < NUM_KEYS; ++i) {
        if (DPS_UUIDCompare(kid, &keyId[i]) == 0) {
            memcpy(key, keyData[i], keyLen);
            return DPS_OK;
        }
    }
    return DPS_ERR_MISSING;
}

static void OnNodeDestroyed(DPS_Node* node, void* data)
{
    if (data) {
        DPS_SignalEvent((DPS_Event*)data, DPS_OK);
    }
}

static void OnPubMatch(DPS_Subscription* sub, const DPS_Publication* pub, uint8_t* data, size_t len)
{
    const DPS_UUID* pubId = DPS_PublicationGetUUID(pub);
    uint32_t sn = DPS_PublicationGetSequenceNum(pub);
    size_t i;
    size_t numTopics = DPS_SubscriptionGetNumTopics(sub);

    if (!quiet) {
        DPS_PRINT("Pub %s(%d) matches:\n    ", DPS_UUIDToString(pubId), sn);
        for (i = 0; i < numTopics; ++i) {
            if (i) {
                DPS_PRINT(" & ");
            }
            DPS_PRINT("%s", DPS_SubscriptionGetTopic(sub, i));
        }
        DPS_PRINT("\n");
        if (data) {
            DPS_PRINT("%.*s\n", (int)len, data);
        }
    }
    if (DPS_PublicationIsAckRequested(pub)) {
        DPS_Status ret = DPS_AckPublication(pub, AckMsg, sizeof(AckMsg));
        if (ret != DPS_OK) {
            DPS_PRINT("Failed to ack pub %s\n", DPS_ErrTxt(ret));
        }
    }
}


static DPS_Status RegisterAndJoin(DPS_Node* node, const char* host, uint16_t port, const char* tenant)
{
    DPS_Status ret;
    DPS_RegistrationList* regs;
    DPS_NodeAddress* remoteAddr = DPS_CreateAddress();

    regs = DPS_CreateRegistrationList(16);

    /*
     * Register with the registration service
     */
    ret = DPS_Registration_PutSyn(node, host, port, tenant);
    if (ret != DPS_OK) {
        DPS_ERRPRINT("Failed to register with registration service: %s\n", DPS_ErrTxt(ret));
        goto Exit;
    }
    /*
     * Find nodes to join
     */
    ret = DPS_Registration_GetSyn(node, host, port, tenant, regs);
    if (ret != DPS_OK) {
        DPS_ERRPRINT("Registration service lookup failed: %s\n", DPS_ErrTxt(ret));
        goto Exit;
    }
    if (regs->count == 0) {
        ret = DPS_ERR_NO_ROUTE;
        goto Exit;
    }
    remoteAddr = DPS_CreateAddress();
    ret = DPS_Registration_LinkToSyn(node, regs, remoteAddr);
    if (ret == DPS_OK) {
        DPS_PRINT("Linked %d to remote node %s\n", DPS_GetPortNumber(node), DPS_NodeAddrToString(remoteAddr));
        goto Exit;
    }

Exit:
    DPS_DestroyAddress(remoteAddr);
    DPS_DestroyRegistrationList(regs);
    return ret;
}

static int IntArg(char* opt, char*** argp, int* argcp, int* val, int min, int max)
{
    char* p;
    char** arg = *argp;
    int argc = *argcp;

    if (strcmp(*arg++, opt) != 0) {
        return 0;
    }
    if (!--argc) {
        return 0;
    }
    *val = strtol(*arg++, &p, 10);
    if (*p) {
        return 0;
    }
    if (*val < min || *val > max) {
        DPS_PRINT("Value for option %s must be in range %d..%d\n", opt, min, max);
        return 0;
    }
    *argp = arg;
    *argcp = argc;
    return 1;
}

int main(int argc, char** argv)
{
    DPS_Status ret;
    DPS_Event* nodeDestroyed;
    char* topics[64];
    char** arg = ++argv;
    const char* tenant = "anonymous_tenant";
    size_t numTopics = 0;
    DPS_Node* node;
    const char* host = "localhost";
    int listen = 0;
    int port = 30000;

    DPS_Debug = 0;

    while (--argc) {
        if (IntArg("-l", &arg, &argc, &listen, 1, UINT16_MAX)) {
            continue;
        }
        if (IntArg("-p", &arg, &argc, &port, 1, UINT16_MAX)) {
            continue;
        }
        if (strcmp(*arg, "-h") == 0) {
            ++arg;
            if (!--argc) {
                goto Usage;
            }
            host = *arg++;
            continue;
        }
        if (strcmp(*arg, "-t") == 0) {
            ++arg;
            if (!--argc) {
                goto Usage;
            }
            tenant = *arg++;
            continue;
        }
        if (strcmp(*arg, "-q") == 0) {
            ++arg;
            quiet = DPS_TRUE;
            continue;
        }
        if (strcmp(*arg, "-d") == 0) {
            ++arg;
            DPS_Debug = 1;
            continue;
        }
        if (*arg[0] == '-') {
            goto Usage;
        }
        if (numTopics == A_SIZEOF(topics)) {
            DPS_PRINT("%s: Too many topics - increase limit and recompile\n", *argv);
            goto Usage;
        }
        topics[numTopics++] = *arg++;
    }

    if (!host || !port) {
        DPS_PRINT("Need host name and port\n");
        goto Usage;
    }

    node = DPS_CreateNode("/.", GetKey, &keyId[0]);

    ret = DPS_StartNode(node, DPS_MCAST_PUB_DISABLED, listen);
    if (ret != DPS_OK) {
        DPS_ERRPRINT("Failed to start node: %s\n", DPS_ErrTxt(ret));
        return 1;
    }
    DPS_PRINT("Subscriber is listening on port %d\n", DPS_GetPortNumber(node));

    nodeDestroyed = DPS_CreateEvent();

    ret = RegisterAndJoin(node, host, port, tenant);
    if (ret != DPS_OK) {
        DPS_PRINT("Failed to link with any other \"%s\" nodes - continuing\n", tenant);
    }

    if (numTopics > 0) {
        DPS_Subscription* subscription = DPS_CreateSubscription(node, (const char**)topics, numTopics);
        ret = DPS_Subscribe(subscription, OnPubMatch);
        if (ret != DPS_OK) {
            DPS_ERRPRINT("Failed to susbscribe topics - error=%s\n", DPS_ErrTxt(ret));
            DPS_DestroyNode(node, OnNodeDestroyed, nodeDestroyed);
        }
    }

    DPS_WaitForEvent(nodeDestroyed);
    DPS_DestroyEvent(nodeDestroyed);
    return 0;

Usage:
    DPS_PRINT("Usage %s [-d] [-l <listen-port>] -h <hostname> -p <portnum> [-t <tenant string>] [-m] topic1 topic2 ... topicN\n", *argv);
    return 1;
}
