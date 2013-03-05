class cTextCommand
{
private:
	struct COMMAND_RESULT * result;
	struct COMMAND_RESULT * next;
	int count;
public:
	cTextCommand();
	~cTextCommand();
	void execute(const char * command);
	const char * nextResult();
	const char * getSetting(const char * key);
	void setSetting(const char * key, const char * value);
	int size();
	const char * get(int i);
};
