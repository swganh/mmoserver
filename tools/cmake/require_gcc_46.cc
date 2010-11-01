#if __GNUC__ < 4
#error
#endif
#if __GNUC__ == 4
#if __GNUC_MINOR__ < 6
#error
#endif
#endif

int main() { return 0; }