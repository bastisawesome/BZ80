#include <QDebug>

#include "version.hpp"

void print_version() {
    qInfo() << "Application Version: " << VERSION << "\n";
    qInfo() << "Full Version: " << FULL_VERSION << "\n";
    qInfo() << "Version Details:\n";
    qInfo() << "Major: " << VERSION_MAJOR << "\n";
    qInfo() << "Minor: " << VERSION_MINOR << "\n";
    qInfo() << "Patch: " << VERSION_PATCH << "\n";
    qInfo() << "Build Date: " << __DATE__ << "\n";
    qInfo() << "Build Time: " << __TIME__ << "\n";
}

int main() {
    qInfo() << "Hello!\n";
    print_version();

    return 0;
}
