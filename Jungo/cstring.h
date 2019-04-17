/* Jungo Connectivity Confidential. Copyright (c) 2019 Jungo Connectivity Ltd.  https://www.jungo.com */

#ifndef _CSTRING_H_
#define _CSTRING_H_

#include <stdarg.h>

typedef const char* PCSTR;
class CCString
{
public:
    // Constructors
    CCString();
    CCString(const CCString& stringSrc);
    CCString(PCSTR pcwStr);

    virtual ~CCString();

    operator PCSTR() const;
    operator char *() const;

    // MFC compatibility functions
    // overloaded assignment
    const int operator==(const CCString& s);
    const int operator==(const char *s);
    const int operator!=(const CCString& s);
    const int operator!=(const char *s);
    const CCString& operator=(const CCString& s);
    const CCString& operator=(PCSTR s);
    const CCString& operator+=(const PCSTR s);
    const CCString& cat_printf(const PCSTR format, ...);
    char& operator[](int i);
    int Compare(PCSTR s);
    int CompareNoCase(PCSTR s);
    const int operator!=(char *s);
    void MakeUpper();
    void MakeLower();

    // sub-string extraction
    CCString Mid(int nFirst, int nCount);
    CCString Mid(int nFirst);

    CCString StrRemove(PCSTR str);
    CCString StrReplace(PCSTR str, PCSTR new_str);

    int Length() const;
    bool contains(PCSTR substr);
    bool is_empty();
    char *m_str; // The String itself

    void Format(const PCSTR format, ...);
    int IsAllocOK();
    int GetBuffer(unsigned long size);

    // ANSI C compatibility functions
    int strcmp(const PCSTR s);
    int stricmp(const PCSTR s);
    void sprintf(const PCSTR format, ...);
    void toupper();
    void tolower();
    CCString tolower_copy(void) const;
    CCString toupper_copy(void) const;
    int find_first(char c) const;
    int find_last(char c) const;
    CCString substr(int start, int end) const;
    CCString trim(int index) const;

protected:
    void Init();
    void vsprintf(const PCSTR format, va_list ap);
    int m_buf_size;
};

const CCString operator+(const CCString &str1, const CCString &str2);

#if !defined(WIN32)
    int stricmp(PCSTR str1, PCSTR str2);
#endif

#endif /* _CSTRING_H_ */

