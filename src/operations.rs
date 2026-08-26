use crate::csv::CSV;
use regex::Regex;

pub fn guess_type(s: String) -> crate::environment::Type {
    crate::environment::Type::infer_type(&s)
}

fn parse_number(s: &str) -> Result<f64, String> {
    let s = s.trim();
    if s.is_empty() {
        return Err("input is empty".to_string());
    }
    s.parse::<f64>()
        .map_err(|_| format!("could not parse '{s}' as a number"))
}

fn is_integer_string(s: &str) -> bool {
    s.trim().parse::<i64>().is_ok()
}

pub fn add_uint(file: &mut CSV, val: u32, col: usize) -> Result<(), String> {
    if col >= file.row_length as usize {
        return Err("error: invalid column index".to_string());
    }

    // Validation pass,  ensure every cell in the column is numeric before
    // touching any of them.
    for row in 0..file.num_rows as usize {
        let index = row * file.row_length as usize + col;
        if index >= file.data.len() {
            return Err(format!("error: row {} is missing column {}", row + 1, col));
        }
        parse_number(&file.data[index])
            .map_err(|e| format!("error: row {} column {}: {}", row + 1, col, e))?;
    }

    // Mutation pass — add val, preserving integer vs float display.
    for row in 0..file.num_rows as usize {
        let index = row * file.row_length as usize + col;
        let cell = &file.data[index];
        let result = parse_number(cell)? + val as f64;

        file.data[index] = if is_integer_string(cell) && result.fract() == 0.0 {
            format!("{}", result as i64)
        } else {
            format!("{}", result)
        };
    }

    Ok(())
}