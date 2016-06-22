// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>

const int iterations = 500000;

static inline long long unsigned time_ns(struct timespec* const ts) {
  if (clock_gettime(CLOCK_REALTIME, ts)) {
    exit(1);
  }
  return ((long long unsigned) ts->tv_sec) * 1000000000LLU
    + (long long unsigned) ts->tv_nsec;
}

ucontext_t main_context, other_context;

void run_other(void) {
	int i;
	for(i=0;i<iterations;++i) {
		swapcontext(&other_context, &main_context);
	}
}

int main(void) {
  struct timespec ts;
	static char other_stack[0x10000];
	assert(getcontext(&other_context) >= 0);	
	other_context.uc_stack.ss_sp = other_stack;
	other_context.uc_stack.ss_size = sizeof(other_stack);
	other_context.uc_link = &main_context;
	makecontext(&other_context, run_other, 1, &main_context);

  const long long unsigned start_ns = time_ns(&ts);
  for (int i = 0; i < iterations; i++) {
		assert(swapcontext(&main_context, &other_context) >= 0);
	}
  const long long unsigned delta = time_ns(&ts) - start_ns;

  const int nswitches = iterations << 2;
  printf("%i user mode context switches in %lluns (%.1fns/ctxsw)\n",
         nswitches, delta, (delta / (float) nswitches));
  return 0;
}
