// EMLang test program
// This is a comment

let x: number = 42;
const message: string = "Hello, EMLang!";

function fibonacci(n: number): number {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

function main(): number {
    let result: number = fibonacci(10);
    
    if (result > 50) {
        return 1;
    } else {
        return 0;
    }
}

let finalResult: number = main();
