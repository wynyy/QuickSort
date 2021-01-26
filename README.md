# <p align="center"> QuickSort implementation in c.</p>
<pre>
Two versions are made, one is multi-process (using fork()).

In both of them recursion is linearised using stack (to reduce memory use and control
overflow).

Stack use explaination :
	- After each loop of regular quick sort, in recursion we usually quick sort the rigth
		side of the pivot, and after the left side (= left before right).
		If we have an really unlucky sort, the right side can have the begin size -1.
		If this append to much (really unlucky case) we could have an overflow.
	- We used a stack as memory to sort the smallest side first.

BubbleSort explaination :
	- The stack have built-in semaphore and mutex. More frequently process acces to the
		stack, more likely 'real' concurency happen.
		To ensure this, for small array we use bubble sort, which are as efficient.

26/01/2021
</pre>
