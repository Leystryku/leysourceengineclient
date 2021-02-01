#pragma once

#include <string>

#include "valve/buf.h"
#include "../deps/asyncgetline.h"

#include "datagram.h"
#include "leychan.h"

void processcmdline(Datagram* datagram, leychan* netchan, std::string sinput)
{
	for (unsigned int i = 0; i < sinput.length(); i++)
	{
		if (sinput[i] == '|')
			sinput[i] = '\n';
	}

	/*
	if (!sinput.substr(0, 5).compare("retry"))
	{
		datagram->Disconnect(&netchan);
		sinput = "";
	}*/

	if (!sinput.substr(0, 10).compare("disconnect"))
	{
		datagram->Disconnect(netchan);
		exit(-1);
		sinput = "";
	}

	if (!sinput.substr(0, 6).compare("setcv "))
	{
		char* cmd = strtok((char*)sinput.c_str(), " ");
		char* cv = strtok(NULL, " ");
		char* var = strtok(NULL, " ");

		printf("Setting convar %s to %s\n", cv, var);

		datagram->Reset();
		bf_write* senddatabuf = netchan->GetSendData();

		senddatabuf->WriteUBitLong(5, 6);
		senddatabuf->WriteByte(1);
		senddatabuf->WriteString(cv);
		senddatabuf->WriteString(var);

		sinput = "";
	}

	if (!sinput.substr(0, 14).compare("file_download "))
	{
		char* cmd = strtok((char*)sinput.c_str(), " ");
		std::string file = sinput.substr(14);

		printf("Requesting file: %s\n", file.c_str());

		datagram->Reset();

		static int requestcount = 100;
		bf_write* senddatabuf = netchan->GetSendData();

		senddatabuf->WriteUBitLong(2, 6);
		senddatabuf->WriteUBitLong(requestcount++, 32);
		senddatabuf->WriteString(file.c_str());
		senddatabuf->WriteOneBit(1);

		sinput = "";
	}

	if (!sinput.substr(0, 12).compare("file_upload "))
	{
		char* cmd = strtok((char*)sinput.c_str(), " ");
		std::string file = sinput.substr(12);


		printf("Uploading file: %s\n", file.c_str());

		datagram->Reset();
		datagram->GenerateLeyFile(netchan, file.c_str(), "hi this is a file upload from leysourceengineclient :)");
		datagram->Send(netchan, true);
		sinput = "";
	}

	if (!sinput.substr(0, 4).compare("cmd "))
	{
		std::string sourcecmd = sinput.substr(4);
		printf("Sending command: %s\n", sourcecmd.c_str());

		bf_write* senddatabuf = netchan->GetSendData();

		senddatabuf->WriteUBitLong(4, 6);
		senddatabuf->WriteString(sourcecmd.c_str());
		sinput = "";
		return;

	}

	if (!sinput.substr(0, 5).compare("name "))
	{
		std::string nickname = sinput.substr(5);

		bf_write* senddatabuf = netchan->GetSendData();

		senddatabuf->WriteUBitLong(5, 6);
		senddatabuf->WriteByte(0x1);
		senddatabuf->WriteString("name");
		senddatabuf->WriteString(nickname.c_str());


		printf("Changing name to: %s\n", nickname.c_str());
		sinput = "";
		return;

	}
}

AsyncGetline ag;
std::string input;

void docmdthink(Datagram* datagram, leychan* netchan)
{
	if (netchan->connectstep)
	{
		return;
	}

	input = ag.GetLine();
	if (input.empty())
	{
		return;
	}

	cout << "{" << input << "}\n";

	processcmdline(datagram, netchan, input);
}