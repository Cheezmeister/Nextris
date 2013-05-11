extern int setup();
extern int update();
extern int cleanup();

void js_update() { update(); }
void js_setup() { setup(); }
void js_cleanup() { cleanup(); }
