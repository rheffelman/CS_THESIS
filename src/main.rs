mod csv;
mod operations;
mod parser;
mod interpreter;
mod environment;

use std::{collections::HashMap, hash::Hash};

use parser::Parser;
use csv::CSV;
use interpreter::Interpreter;

use crate::operations::add_uint;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let p = Parser {
        filepath: "./example_code/1.ry".to_string(),
        contents: None,
        statements: None,
    };

    let mut c = Interpreter {
        data: None,
        parser: p,
        vars: Vec::new(),
        labels: HashMap::new(),
    };

    c.go()?;
    Ok(())
}