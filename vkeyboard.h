#ifndef _C_VKEYBOARD
#define _C_VKEYBOARD

#include <linux/uinput.h>
#include <cstdint>

class CVKeyboard
{
public:
	CVKeyboard(const char* apName);
	~CVKeyboard();
	void Emit(int type, int code, int val);
	void Parse(uint8_t* buf, size_t len);
protected:
private:
	int  digitToCode(uint8_t val);
	int  alphaToCode(uint8_t val);
	int  whitespaceToCode(uint8_t val);
	int  punctToCode(uint8_t val);
	void handleSpecialKey(uint8_t val);

	int  m_fd;
	char m_name[UINPUT_MAX_NAME_SIZE];
	static int m_keys[];
	
	int32_t m_shiftKeyStatus;
	int32_t m_ctrlKeyStatus;
	int32_t m_altKeyStatus;
};

#endif // _C_VKEYBOARD