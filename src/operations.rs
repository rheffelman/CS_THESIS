use crate::csv::CSV;
use regex::Regex;

#[derive(Debug, PartialEq)]
enum Number {
    Int(i64),
    Float(f64),
}
#[derive(Debug, PartialEq)]
pub enum Type {
    Int,
    Float,
    String,
    Bool,
    Unknown,
    Null
}

pub fn guess_type(s: String) -> Type {
    let int_regex = Regex::new(r"^\d{1,10}$").unwrap();
    let float_regex = Regex::new(r"[-]?[0-9]*\.?,?[0-9]+").unwrap();

    if let Some(captures) = int_regex.captures(&s) {
        return Type::Int;
    }
    if let Some(captures) = float_regex.captures(&s) {
        return Type::Float;
    }
    if s == "true".to_string() || s == "false".to_string() {
        return Type::Bool;
    }
    if s.clone().to_lowercase() == "null" {
        return Type::Null;
    }
    return Type::Unknown;
}

fn parse_number(s: String) -> Result<Number, String> {
    let s = s.trim();

    if s.is_empty() {
        return Err("input is empty".to_string());
    }

    if let Ok(n) = s.parse::<i64>() {
        return Ok(Number::Int(n));
    }

    if let Ok(n) = s.parse::<f64>() {
        return Ok(Number::Float(n));
    }

    Err(format!("could not parse '{s}' as an int or float"))
}

pub fn add_uint(file: &mut CSV, val: u32, col: usize) -> Result<(), String> {
    if col >= file.row_length as usize {
        return Err("error: invalid column index".to_string());
    }

    let mut expected_type: Option<&'static str> = None;

    // validation pass, makes sure each value is there and convertible to the same numeric type before continuing with operations.
    for row in 0..file.num_rows as usize {
        let index = row * file.row_length as usize + col;

        if index >= file.data.len() {
            return Err(format!(
                "error: row {} is missing column {}",
                row + 1,
                col
            ));
        }

        match parse_number(file.data[index].clone())? {
            Number::Int(_) => {
                if let Some(t) = expected_type {
                    if t != "int" {
                        return Err(format!(
                            "error: row {} column {} has type int, expected float",
                            row + 1,
                            col
                        ));
                    }
                } else {
                    expected_type = Some("int");
                }
            }
            Number::Float(_) => {
                if let Some(t) = expected_type {
                    if t != "float" {
                        return Err(format!(
                            "error: row {} column {} has type float, expected int",
                            row + 1,
                            col
                        ));
                    }
                } else {
                    expected_type = Some("float");
                }
            }
        }
    }

    // mutation pass
    for row in 0..file.num_rows as usize {
        let index = row * file.row_length as usize + col;

        let new_value = match parse_number(file.data[index].clone())? {
            Number::Int(n) => (n + val as i64).to_string(),
            Number::Float(n) => (n + val as f64).to_string(),
        };

        file.data[index] = new_value;
    }

    Ok(())
}

// pub fn count()