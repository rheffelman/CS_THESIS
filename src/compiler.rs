use crate::{csv::CSV, parser::Parser};
use regex::Regex;
use std::collections::HashMap;
use std::path::Path;

pub struct Compiler {
    pub data: Option<CSV>,
    pub parser: Parser,
}

#[derive(Hash, Eq, PartialEq, Debug)]
enum FunctionKeyword {
    Read,
    Write,
}

impl Compiler {
    pub fn go(&mut self) -> Result<(), Box<dyn std::error::Error>> {
        self.parser.read_file_contents()?;
        self.parser.get_statements()?;
        self.fail_early();

        let keywords = HashMap::from([
            ("read", FunctionKeyword::Read),
            ("write", FunctionKeyword::Write),
        ]);

        let function_regex = Regex::new(
            r"^\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*\((.*)\)\s*$"
        )?;



        if let Some(statements) = self.parser.statements.clone() {
            for statement in statements {
                
                if let Some(captures) = function_regex.captures(&statement) {
                    let keyword_text = captures.get(1).unwrap().as_str();
                    let args_text = captures.get(2).unwrap().as_str();
                    
                    match keywords.get(keyword_text) {
                        Some(FunctionKeyword::Read) => {
                            self.handle_read(&statement, args_text)?;
                        }
                        Some(FunctionKeyword::Write) => {
                            self.handle_write(&statement, args_text)?;
                        }
                        None => {}
                    }
                }
            }
        }

        Ok(())
    }

    fn fail_early(&mut self) -> String{
        if let Some(statements) = self.parser.statements.clone() {
            for statement in statements {
                if !self.check_parentheses_structure(statement.to_string()) {
                    println!("Error: problem with brace or quote structure with statement: {}", statement.to_string());
                    //return ("problem with brace or quote structure with statement: {statement}").to_string();
                }
            }
        }
        return "good".to_string();
    }

    // checks not only parentheses, but {, [, ', and "
    fn check_parentheses_structure(&mut self, input: String) -> bool {
        let mut stack: Vec<char> = Vec::new();
        let mut in_double_quote = false;
        let mut in_single_quote = false;

        for ch in input.chars() {
            if ch == '"' && !in_single_quote {
                in_double_quote = !in_double_quote;
                continue;
            }

            if ch == '\'' && !in_double_quote {
                in_single_quote = !in_single_quote;
                continue;
            }

            if in_double_quote || in_single_quote {
                continue;
            }

            if ch == '(' || ch == '{' || ch == '[' {
                stack.push(ch);
            } else if ch == ')' {
                if stack.pop() != Some('(') {
                    return false;
                }
            } else if ch == '}' {
                if stack.pop() != Some('{') {
                    return false;
                }
            } else if ch == ']' {
                if stack.pop() != Some('[') {
                    return false;
                }
            }
        }

        if in_double_quote || in_single_quote {
            return false;
        }

        if !stack.is_empty() {
            return false;
        }

        return true;
    }
    
    fn handle_read(
        &mut self,
        statement: &str,
        args_text: &str,
    ) -> Result<(), Box<dyn std::error::Error>> {
        let args = self.parse_args(args_text);

        if args.len() != 1 {
            return Err(format!(
                "read() expects exactly 1 argument, got {} in statement: {}",
                args.len(),
                statement
            )
            .into());
        }

        let filepath = &args[0];

        if filepath.trim().is_empty() {
            return Err(format!("read() got an empty filepath in statement: {}", statement).into());
        }

        if !Path::new(filepath).exists() {
            return Err(format!("read() file does not exist: {}", filepath).into());
        }

        let csv = CSV::new(filepath.to_string(), None, true);
        self.data = Some(csv);

        println!("loaded csv from {}", filepath);

        if let Some(data) = &self.data {
            data.print_csv();
        }

        Ok(())
    }
    


    fn handle_write(
        &mut self,
        statement: &str,
        args_text: &str,
    ) -> Result<(), Box<dyn std::error::Error>> {
        let args = self.parse_args(args_text);

        println!("WRITE HANDLER");
        println!("statement: {statement}");
        println!("raw args: {args_text}");
        println!("parsed args: {:?}", args);

        Ok(())
    }

    fn parse_args(&self, args_text: &str) -> Vec<String> {
        let mut args = Vec::new();
        let mut current = String::new();
        let mut in_quotes = false;
        let mut escape = false;

        for ch in args_text.chars() {
            if escape {
                current.push(ch);
                escape = false;
                continue;
            }

            match ch {
                '\\' if in_quotes => {
                    current.push(ch);
                    escape = true;
                }
                '"' => {
                    current.push(ch);
                    in_quotes = !in_quotes;
                }
                ',' if !in_quotes => {
                    let cleaned = self.clean_arg(&current);
                    if !cleaned.is_empty() {
                        args.push(cleaned);
                    }
                    current.clear();
                }
                _ => current.push(ch),
            }
        }

        let cleaned = self.clean_arg(&current);
        if !cleaned.is_empty() {
            args.push(cleaned);
        }

        args
    }

    fn clean_arg(&self, arg: &str) -> String {
        let trimmed = arg.trim();

        if trimmed.starts_with('"') && trimmed.ends_with('"') && trimmed.len() >= 2 {
            trimmed[1..trimmed.len() - 1].to_string()
        } else {
            trimmed.to_string()
        }
    }
}