#include <argparse/argparse.hpp>
#include <server/server.h>
#include <Garnet/Garnet.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <signal.h>
#endif

void cleanup() {
    server::Stop();
    Garnet::Terminate();
}

#ifdef _WIN32
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
    cleanup();
    exit(0);
}
#else
void sighandler(int signum)
{
    cleanup();
    exit(0);
}
#endif

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("lr2arenaex-server");

    program.add_argument("-r", "--rotate")
        .help("enable host auto-rotation")
        .flag();

    program.add_argument("-a", "--address")
        .help("address to listen to")
        .metavar("ADDRESS")
        .default_value(std::string("0.0.0.0"));

    program.add_argument("-p", "--port")
        .help("port to listen to")
        .metavar("PORT")
        .scan<'u', ushort>()
        .default_value(ushort(2222));

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    if (!Garnet::Init(true)) {
		std::cout << "[!] Error initalizing sockets" << std::endl;
		return 1;
	}

    if (program["--rotate"] == true) {
        server::autoRotateHost = true;
        std::cout << "[+] Host auto-rotation enabled" << std::endl;
    }

#ifdef _WIN32
    SetConsoleOutputCP(65001); // Set code page to UTF-8
    SetConsoleCtrlHandler(CtrlHandler, TRUE);
#else
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = sighandler;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);
#endif

    if (!server::Start(program.get<std::string>("--address").c_str(), program.get<ushort>("--port")))
        return 1;
    std::cout << "[+] Press Ctrl+C to stop the server..." << std::endl;
    while (true);

    return 0;
}