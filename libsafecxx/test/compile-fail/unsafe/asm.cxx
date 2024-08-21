#feature on safety

int main() safe {

  // It's unsafe to use inline ASM. The compiler isn't smart enough 
  // to know that it's not unsound.
  volatile size_t timestamp = 0;
  asm("rdtsc\n"          
      "shl $32, %%rdx\n" 
      "or %%rdx, %q0"    
      :  "=a" (timestamp)
      :: "rdx"
  );
}