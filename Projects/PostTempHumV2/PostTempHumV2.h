#ifndef POST_TEMP_HUM
// Declare Handler table and macros for adding handlers
typedef bool(*FunctionPointer)();
extern FunctionPointer *data_collectors; // At time of writing only need to support 2 sensor families.
extern uint8_t HandlerCount=0;
#define RegisterHandler(NAME, HANDLERFUNC) const uint8_t NAME = HandlerCount; data_collectors[HandlerCount++] = HANDLERFUNC

#define POST_TEMP_HUM
#endif
