; ModuleID = 'emlang_module'
source_filename = "emlang_module"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

declare i32 @printf(ptr, ...)

declare i32 @puts(ptr)

declare ptr @malloc(i64)

declare void @free(ptr)

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 42, ptr %x, align 4
  %x1 = load i32, ptr %x, align 4
  ret i32 %x1
  %calltmp = call i32 @main()
}
