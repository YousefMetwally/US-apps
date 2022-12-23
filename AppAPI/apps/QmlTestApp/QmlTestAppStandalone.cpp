#ifdef __ANDROID__
// MobileSandbox main function
extern int main (int argc, char *argv[]);

void DummyMainReference () {
    main(0, nullptr); // prevent main function from being eliminated by the linker
}
#endif
