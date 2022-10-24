#include<string.h>
#include<types.h>


extern "C"{


char *strcpy(char *dest, const char *src){
    char *ptr = dest;
    while(*src != EOS){
        *ptr = *src;
        ptr ++;
        src ++;
    }
    return dest;
};
char *strncpy(char *dest, const char *src, size_t count){
    /*调用方*/
    char *ptr = dest;
    size_t nr = 0;

    for(; nr < count;nr++){
        *ptr = *src;
        if(*src == EOS){
            return dest;
        }
        ptr++;
        src++;
    }
    dest[count - 1] = EOS;
    return dest;   


};
char *strcat(char *dest, const char *src){
    char *ptr = dest;
    while( *ptr != EOS){
        ptr++;
    }
    // 出来的是 EOS 的位置
    while (true){
        *ptr = *src;
        if(*src == EOS){
            return dest;
        }
        ptr++;
        src++;
    }
    

};
size_t strlen(const char *str){
    char *ptr = (char *)str;
    while (*ptr != EOS)
    {
        ptr++;
    }
    return ptr - str;
};
int strcmp(const char *lhs, const char *rhs){
    /*
    1大2 返回 1
    1等2 返回 0
    1小2 返回 -1*/
    while (*lhs == *rhs && *lhs != EOS && *rhs != EOS)
    {
        lhs++;
        rhs++;
    }
    return *lhs < *rhs ? -1 : *lhs > *rhs;
};
char *strchr(const char *str, int ch){
    //某个字符的指针
    char *ptr = (char *)str;
    while (true)
    {
        if (*ptr == ch)
        {
            return ptr;
        }
        if (*ptr == EOS)
        {
            return (char*)NULL;
        }
        ptr++;
    }       

};
char *strrchr(const char *str, int ch){
    /*某个字符的指针,不过是最后一个*/
    char *last = (char *)NULL;
    char *ptr = (char *)str;
    while (true)
    {
        if (*ptr == ch)
        {
            last = ptr;
        }
        if (*ptr == EOS)
        {
            return last;
        }
        ptr++;
    }
};

int memcmp(const void *lhs, const void *rhs, size_t count){
    /*
    1大2 返回 1
    1等2 返回 0
    1小2 返回 -1*/
    char *lptr = (char *)lhs;
    char *rptr = (char *)rhs;
    while (*lptr == *rptr && count-- > 0)
    {
        lptr++;
        rptr++;
    }
    return *lptr < *rptr ? -1 : *lptr > *rptr;
};
void *memset(void *dest, int ch, size_t count){
    char *ptr = (char*)dest;
    while (count--)
    {
        *ptr = ch;
        ptr++;
    }
    return dest;
};
void *memcpy(void *dest, const void *src, size_t count){
    char *ptr = (char *)dest;
    while (count--)
    {
        *ptr = *((char *)(src));
        ptr++;
        src++;

    }
    return dest;
};
void *memchr(const void *str, int ch, size_t count){
    /*
    返回指向某个字符的指针
    */
    char *ptr = (char *)str;
    while (count--)
    {
        if (*ptr == ch)
        {
            return (void *)ptr;
        }
        ptr++;
    }
};

}
