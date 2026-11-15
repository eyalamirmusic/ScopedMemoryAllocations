extern "C" void __libc_free(void*);

using FreeFunc = void (*)(void*);

FreeFunc getBootstrapFree()
{ return __libc_free; }
