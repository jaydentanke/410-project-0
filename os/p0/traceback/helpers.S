/*
 * As you can see, you can perform c style comments in .S files.	
 * You may also use precompiler directives like #include and #define
 */


/* define the add_one1 and add_one2 labels so that they can be called from
 * other files (.c or .S) */
.global get_ebp
.global get_function_addr_from_ebp


/*
    When we are in function
    1. ebp + 4 gives us the return address (r)
    2. r - 4 should give us address of this function

*/ 

# Get ebp without changing anything else
get_ebp:
    movl %ebp, %eax
    ret

get_function_addr_from_ebp:
    pushl %ebp
    movl %esp, %ebp
    call get_ebp
    /* save callee save registers, may not need all */
 	pushl %edi
 	pushl %ebx
 	pushl %esi

    # argument 1: address of frame pointer
    movl 8(%ebp), %edi
    # argument 2: address to store next ebp
    movl 12(%ebp), %esi

    # Store next frame pointer first

    # ebx contains next saved ebp
    movl (%edi), %ebx
    movl %ebx, (%esi)

    # store return address in edi
    movl 4(%edi), %eax

    popl %esi
    popl %ebx
    popl %edi
    movl %ebp, %esp
    popl %ebp
    ret

; /* Method 1 saves off all of the callee save registers
;  * and then adds one to the argument and returns it
;  */
; add_one1:
; 	/* make sure we save all of the callee save registers */
; 	pushl	%ebp		
; 	movl	%esp,%ebp	
; 	pushl	%edi
; 	pushl	%ebx
; 	pushl	%esi
	
;         movl	8(%ebp),%eax	/* get the argument */
;         addl	$1,%eax		/* add one to it */

; 	/* restore the callee save registers */
; 	popl	%esi
; 	popl	%ebx
; 	popl	%edi
; 	movl	%ebp,%esp
; 	popl	%ebp
; 	ret


; /* Method 2 doesn't use any callee save registers or the stack
;  * so it chooses not to save them. Notice the difference in how
;  * we need to get the argument for the function. Figure out why
;  * it is different for the different functions and convince yourself
;  * that it is right.
;  */
; add_one2:
; 	movl	4(%esp),%eax	/* get the argument */
; 	addl	$1,%eax		/* add one to it and return it */
; 	ret


