/* This sample is used to put the ThreadDebug debug monitor through its paces before a release. */

#include <ThreadDebug.h>

//UartDebugCommInterface debugComm(SERIAL1_TX, SERIAL1_RX, 230400);
//ThreadDebug            threadDebug(&debugComm, DEBUG_BREAK_IN_SETUP);

UsbDebugCommInterface  debugComm(&SerialUSB);
ThreadDebug            threadDebug(&debugComm, DEBUG_BREAK_IN_SETUP);

// Assembly language functions found in tests.S
extern "C" void testContextWithCrash(void);
extern "C" void testContextWithHardcodedBreakpoint(void);
extern "C" void testStackingHandlerException(void);

// Forward function declarations.
static void __attribute__ ((noinline)) breakOnMe();
static void runThreads(osPriority_t thread1Priority, osPriority_t thread2Priority);
static void thread1Func(void* pv);
static void thread2Func(void* pv);


void setup() {
    Serial1.begin(230400);
    Serial1.setTimeout(60000);
}

void loop() {
    Serial1.println();
    Serial1.println("1) Set registers to known values and crash.");
    Serial1.println("2) Set registers to known values and stop at hardcoded bkpt.");
    Serial1.println("3) Call breakOnMe() to increment g_global");
    Serial1.println("4) Run 2 threads at normal priority");
    Serial1.println("5) Run 2 threads with testThread2 at osPriorityLow");
    Serial1.println("6) Trigger mbed hard fault handler");


    Serial1.print("Selection: ");

    int selection;
    do {
        selection = Serial1.parseInt();
    } while (selection == 0);
    Serial1.println();

    switch (selection) {
        case 1:
            testContextWithCrash();
            break;
        case 2:
            testContextWithHardcodedBreakpoint();
            break;
        case 3:
            Serial1.println("Delaying 10 seconds...");
            delay(10000);
            breakOnMe();
            break;
        case 4:
            runThreads(osPriorityNormal, osPriorityNormal);
            break;
        case 5:
            runThreads(osPriorityNormal, osPriorityLow);
            break;
        case 6:
            testStackingHandlerException();
            break;
        default:
            Serial1.println("Invalid selection");
            break;
    }
}

static volatile uint32_t g_global;

static void __attribute__ ((noinline)) breakOnMe() {
    g_global++;
    __DSB();
}

static void runThreads(osPriority_t thread1Priority, osPriority_t thread2Priority) {
    delay(250);
    while (Serial1.available()) {
        Serial1.read();
    }
    Serial1.println("Press any key to end test...");

    static uint64_t             thread1Stack[128];
    static osRtxThread_t        thread1Tcb;
    static const osThreadAttr_t thread1Attr =
    {
        .name = "testThread1",
        .attr_bits = osThreadDetached,
        .cb_mem  = &thread1Tcb,
        .cb_size = sizeof(thread1Tcb),
        .stack_mem = thread1Stack,
        .stack_size = sizeof(thread1Stack),
        .priority = thread1Priority
    };
    osThreadId_t thread1 = osThreadNew(thread1Func, NULL, &thread1Attr);

    static uint64_t             thread2Stack[128];
    static osRtxThread_t        thread2Tcb;
    static const osThreadAttr_t thread2Attr =
    {
        .name = "testThread2",
        .attr_bits = osThreadDetached,
        .cb_mem  = &thread2Tcb,
        .cb_size = sizeof(thread2Tcb),
        .stack_mem = thread2Stack,
        .stack_size = sizeof(thread2Stack),
        .priority = thread2Priority
    };
    osThreadId_t thread2 = osThreadNew(thread2Func, NULL, &thread2Attr);

    while (!Serial1.available()) {
        // Wait for user input.
    }
    Serial1.read();

    osThreadTerminate(thread1);
    osThreadTerminate(thread2);
}

static void thread1Func(void* pv) {
    while (true ) {
        delay(1000);
        Serial1.println("Thread1 Output");
    }
}

static void thread2Func(void* pv) {
    while (true ) {
        delay(2000);
        Serial1.println("Thread2 Output");
    }
}
