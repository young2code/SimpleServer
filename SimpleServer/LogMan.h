class LogMan
{
public:
	static const int kMaxSize = 1024;
	typedef void (*RedirectOutputFunc)(const char*);

public:
	static void SetRedirectOutputFunc(RedirectOutputFunc func);
	static void Log(const char* format, ...);

private:
	static RedirectOutputFunc sRedirectOutput;
};

#define LOG(fmt, ...) LogMan::Log(fmt, ##__VA_ARGS__)