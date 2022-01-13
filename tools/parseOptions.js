const commandLineArgs = require('command-line-args');

function parseOptions(optionDefinitions) {
    try {
       return commandLineArgs(optionDefinitions);
    } catch(ex) {
       console.log(ex.message);
       process.exit(-1);
    }
 }

 module.exports = parseOptions;
