#ifndef SMARTALLOC_H
#define SMARTALLOC_H
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef __cplusplus
#include <iostream>
#endif

/* Smartalloc.h       Copyright Clinton Staley 1991
 * 
 * Smartalloc provides an malloc version which checks for several possible
 * errors:
 *
 * 1. Failure of malloc or calloc call for any reason.
 * 2. Attempt to free memory not allocated by malloc, or calloc 
 *    or already freed.
 * 3. Writing past the end or before the beginning of allocated memory.
 * 4. Failure to free memory by some point in the program.
 * 5. Use of freed storage after freeing it.
 * 6. Assumption that data returned by malloc is set to 0.
 *
 * Use smartalloc by including smartalloc.h in any file that calls malloc,
 * calloc, or free.  Also, compile smartalloc.c along with your other .c files.
 * If you make any of errors 1-3 above, smartalloc will report the error
 * and the file/line on which it occured.  To find out if you have left
 * memory unfreed, call report_space().  If any unfreed memory is
 * outstanding, report_space will return the number of bytes of unfreed
 * memory.  If no memory is unfreed, report_space returns 0.  Errors 5
 * and 6 are "detected" by filling the memory block with random bit-patterns, 
 * so that runtime errors are likely to result from those two errors.
 *
 * All rights to this package are reserved by its author.  Duplication of
 * source or object code is permitted only with the author's written
 * consent.
 */

 /*
  * Changes by John Bellardo to merge C and C++ versions into unified
  * source.
  */

#define malloc(x)   smartalloc((x), __FILE__, __LINE__, 0x55)
#define calloc(x,y) smartalloc((x)*(y), __FILE__, __LINE__, 0)
#define free(x)     smartfree((x), __FILE__, __LINE__)
#define valloc(x)	smartvalloc((x), __FILE__, __LINE__, 0x55)
#define realloc(x,y)	smartrealloc((x),(y),0,__FILE__, __LINE__, 0x66)
#define reallocf(x,y)	smartrealloc((x),(y),1,__FILE__, __LINE__, 0x77)

#ifdef __cplusplus
extern "C" {
#endif

void *smartalloc(unsigned long, const char *, int, char);
void *smartrealloc(void*, unsigned long, int, const char *, int, char);
void *smartvalloc(unsigned long, const char *, int, char);
void smartfree(void *,const  char *, int);
unsigned long report_space();
void smartalloc_track(char *data, unsigned long space, unsigned char needs_free, unsigned short margin);


#ifdef __cplusplus
}
#endif



#ifdef __cplusplus

#include <new>

inline void *operator new(size_t size, char *file, int line, char pat) { return smartalloc(size, file, line, pat); }
inline void *operator new[](size_t size, char *file, int line, char pat) { return smartalloc(size, file, line, pat); }
inline void operator delete(void *p) { smartfree(p, __FILE__, __LINE__); }
inline void operator delete[](void *p) { smartfree(p, __FILE__, __LINE__); }
inline void *operator new(size_t size, const std::nothrow_t&t, char *file, int line, char pat) { return smartalloc(size, file, line, pat); }
inline void *operator new[](size_t size, const std::nothrow_t&t, char *file, int line, char pat) { return smartalloc(size, file, line, pat); }
inline void *operator new(size_t size) { return smartalloc(size, __FILE__, __LINE__, 0x54); }

inline void *operator new(size_t size, const std::nothrow_t&t) { return smartalloc(size, __FILE__, __LINE__, 0x54); }
inline void *operator new[](size_t size, const std::nothrow_t&t) { return smartalloc(size, __FILE__, __LINE__, 0x54); }

inline void *operator new(size_t size, const void *p, const char *file, int line, char pat)
{
	char *data = (char*)p;

	memset(data, 0x54, size);
	smartalloc_track(data, size, 0, 0);

	return data;
}
inline void *operator new[](size_t size, const void *p, const char *file, int line, char pat)
{
	char *data = (char*)p;

	memset(data, 0x54, size);
	smartalloc_track(data, size, 0, 0);

	return data;
}

#define new(...) new(__VA_ARGS__, __FILE__, __LINE__, 0x54)
// #define new new(__FILE__, __LINE__, 0x54)

template <class T>
class STLsmartalloc
{
	public:
		typedef T                 value_type;
		typedef value_type*       pointer;
		typedef const value_type* const_pointer;
		typedef value_type&       reference;
		typedef const value_type& const_reference;
		typedef std::size_t       size_type;
		typedef std::ptrdiff_t    difference_type;

		STLsmartalloc() {}
		STLsmartalloc(const STLsmartalloc &) {}
		~STLsmartalloc() {}

		template <class U> STLsmartalloc(const STLsmartalloc<U>&) {}

		pointer address(reference x) const { return &x; }
		const_pointer address(const_reference x) const { return &x; }
		void construct(pointer p, const value_type& x) { new(p) value_type(x); }
		void destroy(pointer p)
		{
			p->~value_type();
			smartfree(p,__FILE__, __LINE__);
		}

		void deallocate(pointer p, size_type) { smartfree(p, __FILE__, __LINE__); }
		pointer allocate(size_type n, const_pointer = 0)
		{
			void* p = smartalloc(n * sizeof(T), __FILE__, __LINE__, 0x55);
			if (!p)
				throw std::bad_alloc();
			return static_cast<pointer>(p);
		}
		size_type max_size() const
		{
			return static_cast<size_type>(-1) / sizeof(value_type);
		}

		template <class U> struct rebind { typedef STLsmartalloc<U> other; };

	private:
		void operator=(const STLsmartalloc&);
};

template<> class STLsmartalloc<void>
{
	typedef void        value_type;
	typedef void*       pointer;
	typedef const void* const_pointer;

	template <class U> struct rebind { typedef STLsmartalloc<U> other; };
};

template <class T>
inline bool operator==(const STLsmartalloc<T>&, const STLsmartalloc<T>&)
{
	return true;
}

template <class T>
inline bool operator!=(const STLsmartalloc<T>&, const STLsmartalloc<T>&)
{
	return false;
}

// #define SMA(x) STLsmartalloc<x,__FILE__,__LINE__> 
#define SMA(x) STLsmartalloc<x> 

#endif



#endif
