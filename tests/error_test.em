// Test file with semantic errors

let x: number = 42;
let x: string = "duplicate"; // Error: Duplicate variable in same scope

function test(a: number): string {
    let y: number = a + "hello"; // Error: Type mismatch
    if (x) { // Error: condition is not boolean
        return y; // Error: return type mismatch
    }
    return a; // Error: return type mismatch
}

let result: string = undeclaredVar; // Error: Undefined identifier

function duplicate() : number { return 1; }
function duplicate() : string { return "hello"; } // Error: Duplicate function

let z: boolean = 5 + true; // Error: Type mismatch in binary operation
