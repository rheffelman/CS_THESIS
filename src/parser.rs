use std::fs;
use std::io;
use regex::Regex;

pub struct Parser {
    pub filepath: String,
    pub contents: Option<String>,
    pub statements: Option<Vec<String>>,
}

impl Parser {
    pub fn read_file_contents(&mut self) -> io::Result<()> {
        let raw = fs::read_to_string(&self.filepath)?;
        self.contents = Some(raw);
        Ok(())
    }

    pub fn get_statements(&mut self) -> io::Result<()> {
        let contents = match &self.contents {
            Some(c) => c,
            None => {
                return Err(io::Error::new(
                    io::ErrorKind::InvalidData,
                    "contents is None",
                ));
            }
        };

        let trailing_check = Regex::new(r";\s*$").unwrap();
        if !trailing_check.is_match(contents) {
            return Err(io::Error::new(
                io::ErrorKind::InvalidData,
                "non-whitespace content found after last ';'",
            ));
        }

        let split_regex = Regex::new(r";").unwrap();
        let statements: Vec<String> = split_regex
            .split(contents)
            .map(|s| s.trim())
            .filter(|s| !s.is_empty())
            .map(|s| s.to_string())
            .collect();

        self.statements = Some(statements);
        Ok(())
    }

    pub fn print_statements(&self) {
        match &self.statements {
            Some(statements) => {
                for (i, statement) in statements.iter().enumerate() {
                    println!("{}: {}", i, statement);
                }
            }
            None => {
                println!("No statements loaded.");
            }
        }
    }
}