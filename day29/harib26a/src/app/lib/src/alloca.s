.arch i486

.text
.global __alloca
__alloca:
    addl $-4, %eax
    subl %eax, %esp
    jmp *(%esp,%eax,1)

