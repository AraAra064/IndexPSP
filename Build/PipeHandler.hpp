#include <string>
#include <windows.h>

#ifndef _PIPEMSG
#define _PIPEMSG

namespace pmsg
{

enum class PipeMode{RECV, SND};
enum class PipeMsgState{SUCCESS, FAIL};

std::string ErrToStr(PipeMsgState e, uint32_t e1)
{
	std::string retVal = "No error.";

	switch (e)
	{
		case PipeMsgState::SUCCESS:
			break;
		case PipeMsgState::FAIL: //Generic error
			retVal = "An error occured. GetLastError(): e";
			break;
	}

	return retVal;
}

class PipeMsg
{
	HANDLE pipeHandle;
	PipeMode pm;
	std::string pipeName;
	PipeMsgState state;
	uint32_t lastErr;

	protected:
		void init(std::string name, PipeMode mode)
		{
			pipeName = name;
			pm = mode;

			if (mode == PipeMode::RECV)
			{
				pipeHandle = CreateNamedPipeA(name.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 1024, 1024, 0, NULL);
				if (pipeHandle == INVALID_HANDLE_VALUE)
				{
					state = PipeMsgState::FAIL;
					lastErr = GetLastError();
					return;
				}

				if (!ConnectNamedPipe(pipeHandle, NULL))
				{
					state = PipeMsgState::FAIL;
					lastErr = GetLastError();
					return;
				}
				//Can now recv
			}
			else
			{
				pipeHandle = CreateFileA(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
				if (pipeHandle == INVALID_HANDLE_VALUE)
				{
					state = PipeMsgState::FAIL;
					lastErr = GetLastError();
					return;
				}
			}

			state = PipeMsgState::SUCCESS;
			return;
		}

		void _send(std::string data)
		{
			DWORD si = 0;
			bool s = WriteFile(pipeHandle, &data[0], data.size(), &si, NULL);
			//std::cout << "SEND " << si << " bytes" << std::endl;
			if (!s)
			{
				state = PipeMsgState::FAIL;
				lastErr = GetLastError();
			}
			return;
		}
		void _recv(std::string& data)
		{
			DWORD si = 0;
			data.resize(2048);
			bool s = ReadFile(pipeHandle, &data[0], 2048, &si, NULL);
			//std::cout << "SEND " << si << " bytes" << std::endl;
			if (!s)
			{
				state = PipeMsgState::FAIL;
				lastErr = GetLastError();
			}
			return;
		}
	public:
		PipeMsg()
		{
			state = PipeMsgState::FAIL;
			lastErr = -1;
		}
		PipeMsg(std::string pipeName, PipeMode pipeMode)
		{
			init(pipeName, pipeMode);
		}
		~PipeMsg()
		{
			//stuff
			CloseHandle(pipeHandle);
		}

		bool initalise(std::string pipeName, PipeMode pipeMode)
		{
			init(pipeName, pipeMode);
			return state == PipeMsgState::SUCCESS;
		}

		void send(std::string data)
		{
			if (pm == PipeMode::SND && state == PipeMsgState::SUCCESS)
			{
				_send(data);
			}
			return;
		}
		void recv(std::string &data)
		{
			if (pm == PipeMode::RECV && state == PipeMsgState::SUCCESS)
			{
				_recv(data);
			}
			return;
		}

		uint32_t getErr(void){return lastErr;
		}
		bool isValid(void){return lastErr == 0;
		}
		std::string getPipeName(void){return pipeName;
		}
};

}

#endif // !_PIPEMSG

