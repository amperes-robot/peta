#include "io.h"
#include "loop.cpp"
#include <mutex>
#include <thread>
#include <queue>
#include <algorithm>
#include <string>

std::mutex strbuf_m;

void poll(std::queue<std::string>* q, bool* quit)
{
	while (!*quit)
	{
		std::string s;
		std::cin >> s;
		if (s == "") continue;

		{
			std::lock_guard<std::mutex> guard(strbuf_m);
			q->push(s);
		}

		if (s == "QUIT")
		{
			return;
		}
	}
}

void handle_events(bool& quit, std::queue<std::string>& strbuf)
{
	std::lock_guard<std::mutex> guard(strbuf_m);
	while (!strbuf.empty())
	{
		auto s = strbuf.front();
		strbuf.pop();

		if (s == "QUIT")
		{
			quit = true;
		}

		if (s.length() != 4)
		{
			io::log<io::CRITICAL>("Malformed packet received");
			continue;
		}

		if (s[0] == 'I')
		{
			int pin = (int) s[1];
			int data = (((int) s[2]) << 8) + (int) s[3];

			io::set_input(pin, data);
		}
	}
}

int main()
{
	bool quit = false;
	std::queue<std::string> strbuf;
	std::thread io_t(poll, &strbuf, &quit);

	try
	{
		init_c();
		for (;;)
		{
			loop_c();
			handle_events(quit, strbuf);
		}
	}
	catch (...)
	{
		quit = true;
	}

	std::cout << "QUIT" << std::endl;
	return 0;
}
