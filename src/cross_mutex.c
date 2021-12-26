/*
 * Copyright (c) 2018-2022 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "cross_mutex.h"

int MUTEX_INIT(MUTEX *mutex)
{
#ifdef _WIN32
	*mutex = CreateMutex(0, FALSE, 0);;
	return (*mutex==0);
#elif __APPLE__
#elif __unix__
	return pthread_mutex_init (mutex, NULL);
#else
	return -1;
#endif
}

int MUTEX_LOCK(MUTEX *mutex)
{
#ifdef _WIN32
	return (WaitForSingleObject(*mutex, INFINITE)==WAIT_FAILED?1:0);
#elif __APPLE__
#elif __unix__
	return pthread_mutex_lock( mutex );
#else
	return -1;
#endif
}

int MUTEX_UNLOCK(MUTEX *mutex)
{
#ifdef _WIN32
	return (ReleaseMutex(*mutex)==0);
#elif __APPLE__
#elif __unix__
	return pthread_mutex_unlock( mutex );
#else
	return -1;
#endif
}

int MUTEX_DESTROY(MUTEX *mutex)
{
#ifdef _WIN32
	return (CloseHandle(*mutex)==0);
#elif __APPLE__
#elif __unix__
	return pthread_mutex_destroy(mutex);
#else
	return -1;
#endif
}
