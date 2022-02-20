#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdexcept>
#include <cctype>
#include "vkeyboard.h"

// list of all keys for the device to support
int CVKeyboard::m_keys[] = {
	KEY_ESC       , KEY_1        ,KEY_2         ,KEY_3         ,KEY_4         ,KEY_5         ,KEY_6         ,KEY_7,
	KEY_8         , KEY_9        ,KEY_0         ,KEY_MINUS     ,KEY_EQUAL     ,KEY_BACKSPACE ,KEY_TAB       ,KEY_Q,
	KEY_W         , KEY_E        ,KEY_R         ,KEY_T         ,KEY_Y         ,KEY_U         ,KEY_I         ,KEY_O,
	KEY_P         , KEY_LEFTBRACE,KEY_RIGHTBRACE,KEY_ENTER     ,KEY_LEFTCTRL  ,KEY_A         ,KEY_S         ,KEY_D,
	KEY_F         , KEY_G        ,KEY_H         ,KEY_J         ,KEY_K         ,KEY_L         ,KEY_SEMICOLON ,KEY_APOSTROPHE,
	KEY_GRAVE     , KEY_LEFTSHIFT,KEY_BACKSLASH ,KEY_Z         ,KEY_X         ,KEY_C         ,KEY_V         ,KEY_B,
	KEY_N         , KEY_M        ,KEY_COMMA     ,KEY_DOT       ,KEY_SLASH     ,KEY_RIGHTSHIFT,KEY_KPASTERISK,KEY_LEFTALT,
	KEY_SPACE     , KEY_CAPSLOCK ,KEY_F1        ,KEY_F2        ,KEY_F3        ,KEY_F4        ,KEY_F5        ,KEY_F6,
	KEY_F7        , KEY_F8       ,KEY_F9        ,KEY_F10       ,KEY_NUMLOCK   ,KEY_SCROLLLOCK,KEY_KP7       ,KEY_KP8,
	KEY_KP9       , KEY_KPMINUS  ,KEY_KP4       ,KEY_KP5       ,KEY_KP6       ,KEY_KPPLUS    ,KEY_KP1       ,KEY_KP2,
	KEY_KP3       , KEY_KP0      ,KEY_KPDOT     ,KEY_F11       ,KEY_F12       ,KEY_KPENTER   ,KEY_RIGHTCTRL ,KEY_RIGHTALT,
	KEY_LINEFEED  , KEY_HOME     ,KEY_UP        ,KEY_PAGEUP    ,KEY_LEFT      ,KEY_RIGHT     ,KEY_END       ,KEY_DOWN,
	KEY_PAGEDOWN  , KEY_INSERT   ,KEY_DELETE
};

CVKeyboard::CVKeyboard(const char* apName):
				m_shiftKeyStatus(0),
				m_ctrlKeyStatus(0),
				m_altKeyStatus(0)
{
	struct uinput_setup usetup;
	
	strncpy(m_name, apName, sizeof(m_name));
	m_name[sizeof(m_name)-1] = '\0';
	m_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if(m_fd < 0)
	{
		throw std::runtime_error("failed openning /dev/uinput");
	} 
	ioctl(m_fd, UI_SET_EVBIT, EV_KEY); // add key event
	for (int i = 0; i < sizeof(m_keys)/sizeof(m_keys[0]); i++)
	{
        ioctl(m_fd, UI_SET_KEYBIT, m_keys[i]); // register all wanted keys
    }
	
	memset(&usetup, 0x0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1209; // Generic vendor
    usetup.id.product = 0x5678;
    strcpy(usetup.name, m_name);
    ioctl(m_fd, UI_DEV_SETUP, &usetup); // setup the device
    ioctl(m_fd, UI_DEV_CREATE); // create the device
	sleep(5);
	
	char name[16]={0};
	ioctl(m_fd, UI_GET_SYSNAME(sizeof(name)), name);
	std::cout << "device name is " << name << std::endl;
}

CVKeyboard::~CVKeyboard()
{
	if(m_fd > 0)
	{
		ioctl(m_fd, UI_DEV_DESTROY);
		close(m_fd);
	}
}

// based on example code from https://www.kernel.org/doc/html/v4.12/input/uinput.html
void CVKeyboard::Emit(int type, int code, int val)
{
	struct input_event ie;

	ie.type = type;
	ie.code = code;
	ie.value = val;
	/* timestamp values below are ignored */
	ie.time.tv_sec = 0;
	ie.time.tv_usec = 0;

	write(m_fd, &ie, sizeof(ie));
	usleep(40000);
}

int CVKeyboard::digitToCode(uint8_t val)
{
	if(val == '0')
	{
		return KEY_0;
	}

	return val - '1' + KEY_1;
}

int CVKeyboard::alphaToCode(uint8_t val)
{
	std::string row0 = "qwertyuiop";
	std::string row1 = "asdfghjkl";
	std::string row2 = "zxcvbnm";
	size_t pos = std::string::npos;
	
	if((pos = row0.find(val)) != std::string::npos)
	{
		return KEY_Q + pos;
	}
	
	if((pos = row1.find(val)) != std::string::npos)
	{
		return KEY_A + pos;
	}
	
	if((pos = row2.find(val)) != std::string::npos)
	{
		return KEY_Z + pos;
	}
	
	return KEY_A;
}

int CVKeyboard::whitespaceToCode(uint8_t val)
{
	switch(val)
	{
		case '\n':
			return KEY_LINEFEED;
		case '\r':
			return KEY_ENTER;
		case '\t':
			return KEY_TAB;
		default:
			return KEY_SPACE;
	}
}

int CVKeyboard::punctToCode(uint8_t val)
{
	switch(val)
	{
		case '\'':
			return KEY_APOSTROPHE;
		case '*':
			return KEY_KPASTERISK;
		case '+':
			return KEY_KPPLUS;
		case '-':
			return KEY_MINUS;
		case '.':
			return KEY_DOT;
		case '/':
			return KEY_SLASH;
		case ';':
			return KEY_SEMICOLON;
		case '=':
			return KEY_EQUAL;
		case '\\':
			return KEY_BACKSLASH;
		case '`':
			return KEY_GRAVE;
		case '{':
			return KEY_LEFTBRACE;
		case '}':
			return KEY_RIGHTBRACE;
		default:
			return KEY_COMMA;
	}
}

void CVKeyboard::handleSpecialKey(uint8_t val)
{
	int code = KEY_RESERVED;
	int pressed = 1;
	int *keyVal = &pressed;
	bool isToggle = false;
	
	switch(val)
	{
		case 'S':
			code = KEY_LEFTSHIFT;
			keyVal = &m_shiftKeyStatus;
			isToggle = true;
			break;
		case 'C':
			code = KEY_LEFTCTRL;
			keyVal = &m_ctrlKeyStatus;
			isToggle = true;
			break;
		case 'A':
			code = KEY_LEFTALT;
			keyVal = &m_altKeyStatus;
			isToggle = true;
			break;
		case 'E':
			code = KEY_ESC;
			break;
		case 'Z':
			code = KEY_DELETE;
			break;
		case 'B':
			code = KEY_BACKSPACE;
			break;
		case 'U':
			code = KEY_UP;
			break;
		case 'D':
			code = KEY_DOWN;
			break;
		case 'L':
			code = KEY_LEFT;
			break;
		case 'R':
			code = KEY_RIGHT;
			break;

		default:
			return;
	}

	if(isToggle)
	{
		*keyVal = !(*keyVal);
		Emit(EV_KEY, code, *keyVal);
	}
	else
	{
		Emit(EV_KEY, code, 1);
		Emit(EV_SYN, SYN_REPORT, 0);
		usleep(400);
		Emit(EV_KEY, code, 0);
	}
	Emit(EV_SYN, SYN_REPORT, 0);
}

void CVKeyboard::Parse(uint8_t* buf, size_t len)
{
	int code = KEY_RESERVED;
	for(size_t i = 0; i < len; i++)
	{
		if(isdigit(buf[i]))
		{
			code = digitToCode(buf[i]);
		}
		
		if(isalpha(buf[i]))
		{
			if(islower(buf[i]))
			{
				code = alphaToCode(buf[i]);
			}
			else
			{
				handleSpecialKey(buf[i]);
				continue;
			}
		}
		
		if(isspace(buf[i]))
		{
			code = whitespaceToCode(buf[i]);
		}
		
		if(ispunct(buf[i]))
		{
			code = punctToCode(buf[i]);
		}
		
		Emit(EV_KEY, code, 1);
		Emit(EV_KEY, code, 0);
		Emit(EV_SYN, SYN_REPORT, 0);
	}		
}