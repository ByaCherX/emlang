// EMLang test program
// This is a comment

let x: int32 = 42;

function fibonacci(n: int32): int32 {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

function main(): int32 {
    let myNum = 10;
    let result: int32 = fibonacci(myNum);
    
    if (result > 50) {
        return 1;
    } else {
        return 0;
    }
}

let finalResult: int32 = main();

