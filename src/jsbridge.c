extern int setup();
extern int update();
extern int cleanup();

extern "C" void js_setup()   {   setup(); }
extern "C" void js_update()  {  update(); }
extern "C" void js_cleanup() { cleanup(); }
