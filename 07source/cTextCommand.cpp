#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "include/tunnel-client.h"
#include "cTextCommand.h"

cTextCommand::cTextCommand()
{
	this->result = NULL;
	this->next = NULL;
}

cTextCommand::~cTextCommand()
{
	FreeCommandResult(this->result);
}

void
cTextCommand::execute(const char * command)
{
	FreeCommandResult(this->result);
	this->result = TextCommand(command);
	this->next = this->result;
}

const char *
cTextCommand::nextResult()
{
	COMMAND_RESULT * next = this->next;
	if (next != NULL) {
		this->next = this->next->next;
		return next->text;
	}
	return NULL;
}

const char *
cTextCommand::getSetting(const char * key)
{
	char * tmp = (char *)malloc(strlen(key)+10);
	snprintf(tmp, strlen(key)+10, "/get %s", key);
	this->execute(tmp);
	free(tmp);
	if (this->size() < 1) {
		return "";
	}
	return nextResult();
}

void
cTextCommand::setSetting(const char * key, const char * value)
{
	char * tmp = (char *)malloc(strlen(key)+strlen(value)+10);
	snprintf(tmp, strlen(key)+strlen(value)+10, "/set %s %s", key, value);
	this->execute(tmp);
	free(tmp);
}

int
cTextCommand::size()
{
	int count = 0;
	COMMAND_RESULT * next = this->result;
	while (next != NULL)
	{
		next = next->next;
		count++;
	}
	return count;		
}

const char *
cTextCommand::get(int i)
{
	int count = 0;
	COMMAND_RESULT * next = this->result;
	while (next != NULL)
	{
		if (count == i) {
			return next->text;
		}
		next = next->next;
		count++;
	}
	return NULL;		
}



