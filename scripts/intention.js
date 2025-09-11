// Compiled by EasyJS version 0.4.2
let results = [];
console.log('Started, press Ctrl+C to stop');

 
        const interval = setInterval(() => {
let choice = Math.floor(Math.random() * (1 - 0 + 1) + 0);
results.push(choice);
},10); 
    ;
Deno.addSignalListener('SIGINT',() => {
clearInterval(interval);
console.log('\nKeyboardInterrupt caught!');
let zeros = results.filter((x) => {
return x == 0;
}).length;
let ones = results.filter((x) => {
return x == 1;
}).length;
console.log(`Number of zeros: ${zeros}`);
console.log(`Number of ones: ${ones}`);
if (zeros > ones) {
console.log('Zeros win');
}else if (ones > zeros) {
console.log('Ones win');
}else { 
console.log("It's a tie!");
};
Deno.exit(0);
});
