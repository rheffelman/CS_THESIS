mod csv;
mod operations;
mod parser;
mod compiler;

use parser::Parser;
use csv::CSV;
use compiler::Compiler;

use crate::operations::add_uint;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let p = Parser {
        filepath: "./example_code/2.ry".to_string(),
        contents: None,
        statements: None,
    };

    let mut c = Compiler {
        data: None,
        parser: p,
    };

    c.go()?;
    Ok(())
}