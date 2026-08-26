use crate::{csv::CSV, parser::Parser, environment::Variable};
use regex::Regex;
use std::collections::HashMap;
use std::path::Path;

pub struct Interpreter {
    pub data: Option<CSV>,
    pub parser: Parser,
    pub vars: Vec<Variable>,
    pub labels: HashMap<String, usize>,
}

impl Interpreter {
    pub fn go(&mut self) -> Result<(), Box<dyn std::error::Error>> {
        self.parser.read_file_contents()?;
        self.parser.get_statements()?;
        self.fail_early();
        self.get_labels();

        if let Some(statements) = self.parser.statements.clone() {
            let mut index = 0;

            while index < statements.len() {
                let statement = statements[index].trim().to_string();
                let keyword = Self::leading_keyword(&statement);

                match keyword.as_deref() {
                    Some("label") => {
                        index += 1;
                    }
                    Some("read") => {
                        self.handle_read(&statement)?;
                        index += 1;
                    }
                    Some("write") => {
                        self.handle_write(&statement)?;
                        index += 1;
                    }
                    Some("let") => {
                        self.handle_let(&statement)?;
                        index += 1;
                    }
                    Some("print") => {
                        self.handle_print(&statement)?;
                        index += 1;
                    }
                    Some("if") => {
                        if let Some(new_index) = self.handle_if(&statement, &statements)? {
                            index = new_index;
                        } else {
                            index += 1;
                        }
                    }
                    Some("branch") => {
                        let label_name = Self::parse_branch_target(&statement)
                            .ok_or_else(|| format!("Malformed branch statement: {}", statement))?;
                        let label_index = self.labels.get(&label_name)
                            .copied()
                            .ok_or_else(|| format!("Label '{}' not found.", label_name))?;
                        index = label_index;
                    }
                    Some("assign") => {
                        self.handle_assign(&statement)?;
                        index += 1;
                    }
                    _ => {
                        return Err(format!("Unrecognized statement: {}", statement).into());
                    }
                }
            }
        }
        Ok(())
    }

    fn dereference(&mut self, obj: String) -> String {

        
        for var in self.vars.clone() {
            if var.name == obj {
                let mut ret_str = String::new();
                ret_str += &("{NAME: ".to_string() + &var.name + "}");
                ret_str += &("TYPE: ".to_string() + crate::environment::Type::as_str(&var.var_type) + "}");
                return ret_str;
            }
        }
        "".to_string()
    }

    fn leading_keyword(statement: &str) -> Option<String> {
        let keyword_re = Regex::new(r"(?i)^\s*(read|write|let|print|branch|label|if)\b").unwrap();
        if let Some(kw) = keyword_re
            .captures(statement)
            .and_then(|c| c.get(1))
            .map(|m| m.as_str().to_lowercase())
        {
            return Some(kw);
        }

        let assign_re = Regex::new(r"^\s*\w+\s*=[^=]").unwrap();
        if assign_re.is_match(statement) {
            return Some("assign".to_string());
        }

        None
    }

    fn parse_branch_target(statement: &str) -> Option<String> {
        let re = Regex::new(r"(?i)^\s*branch\s+(\w+)\s*[;:]?\s*$").unwrap();
        re.captures(statement)
            .and_then(|c| c.get(1))
            .map(|m| m.as_str().to_string())
    }

    fn fail_early(&mut self) {
        if let Some(statements) = self.parser.statements.clone() {
            for statement in &statements {
                if !self.check_parentheses_structure(statement) {
                    eprintln!(
                        "Error: unbalanced braces/quotes in statement: {}",
                        statement
                    );
                }
            }
        }
    }

    fn check_parentheses_structure(&self, input: &str) -> bool {
        let mut stack: Vec<char> = Vec::new();
        let mut in_double_quote = false;
        let mut in_single_quote = false;

        for ch in input.chars() {
            match ch {
                '"' if !in_single_quote => in_double_quote = !in_double_quote,
                '\'' if !in_double_quote => in_single_quote = !in_single_quote,
                _ if in_double_quote || in_single_quote => {}
                '(' | '{' | '[' => stack.push(ch),
                ')' => { if stack.pop() != Some('(') { return false; } }
                '}' => { if stack.pop() != Some('{') { return false; } }
                ']' => { if stack.pop() != Some('[') { return false; } }
                _ => {}
            }
        }

        !in_double_quote && !in_single_quote && stack.is_empty()
    }

    fn get_labels(&mut self) {
        let re = Regex::new(r"(?i)^\s*label\s+(\w+)\s*[;:]?\s*$").unwrap();

        if let Some(statements) = &self.parser.statements {
            for (index, statement) in statements.iter().enumerate() {
                if let Some(captures) = re.captures(statement) {
                    if let Some(label) = captures.get(1) {
                        self.labels.insert(label.as_str().to_string(), index);
                    }
                }
            }
        }
    }

    fn handle_read(&mut self, statement: &str) -> Result<(), Box<dyn std::error::Error>> {
        let args_text = Self::extract_args(statement)?;
        let args = self.parse_args(&args_text);

        if args.len() != 1 {
            return Err(format!(
                "read() expects exactly 1 argument, got {}: {}",
                args.len(), statement
            ).into());
        }

        let filepath = &args[0];
        if filepath.trim().is_empty() {
            return Err(format!("read() got an empty filepath: {}", statement).into());
        }
        if !Path::new(filepath).exists() {
            return Err(format!("read() file does not exist: {}", filepath).into());
        }

        let csv = CSV::new(filepath.to_string(), None, true);
        self.data = Some(csv);
        println!("Loaded CSV from {}", filepath);
        if let Some(data) = &self.data {
            data.print_csv();
        }
        Ok(())
    }

    fn handle_write(&mut self, statement: &str) -> Result<(), Box<dyn std::error::Error>> {
        let args_text = Self::extract_args(statement)?;
        let args = self.parse_args(&args_text);
        println!("WRITE | statement: {} | args: {:?}", statement, args);
        Ok(())
    }

    fn handle_let(&mut self, statement: &str) -> Result<(), Box<dyn std::error::Error>> {
        let re = Regex::new(r##"(?i)^let\s+(\w+)\s*=\s*("[^"]*"|'[^']*'|`[^`]*`|[\w.]+)$"##).unwrap();
        let cap = re
            .captures(statement.trim())
            .ok_or_else(|| format!("Invalid let statement: {}", statement))?;

        let var_name = cap[1].to_string();
        let raw_value = cap[2].to_string();

        // Alias an existing variable.
        if let Some(existing) = self.vars.iter().find(|v| v.name == raw_value).cloned() {
            let mut new_var = existing;
            new_var.name = var_name;
            self.vars.push(new_var);
            return Ok(());
        }

        let inferred = crate::environment::Type::infer_type(&raw_value);
        let new_var = match inferred {
            crate::environment::Type::Numeric => {
                Self::make_numeric_var(var_name, &raw_value)?
            }
            crate::environment::Type::String => {
                Variable::from_string(var_name, &raw_value[1..raw_value.len() - 1])
            }
            crate::environment::Type::Bool => {
                Variable::from_bool(var_name, raw_value.eq_ignore_ascii_case("true"))
            }
            crate::environment::Type::Null => Variable::null(var_name),
            crate::environment::Type::Vector => {
                let parts: Vec<&str> = raw_value.split(',').collect();
                Variable::from_vector(var_name, &parts)
            }
            crate::environment::Type::Unknown => {
                Variable::unknown(var_name, raw_value.into_bytes())
            }
        };

        self.vars.push(new_var);
        Ok(())
    }

    fn handle_print(&mut self, statement: &str) -> Result<(), Box<dyn std::error::Error>> {
        let args_text = Self::extract_args(statement)?;
        let args = self.parse_args(&args_text);

        if args.is_empty() {
            return Err(format!("print() expects at least 1 argument: {}", statement).into());
        }

        let output: Vec<String> = args.iter().map(|arg| {
            if let Some(var) = self.vars.iter().find(|v| v.name == *arg) {
                format!("{} ({:?})", var, var.var_type)
            } else {
                arg.clone()
            }
        }).collect();

        println!("{}", output.join(" "));
        Ok(())
    }

    fn handle_if(&mut self, statement: &str, statements: &[String]) -> Result<Option<usize>, Box<dyn std::error::Error>> {
        let re = Regex::new(r"(?i)^\s*if\s*\((.+)\)\s*branch\s+(\w+)\s*[;:]?\s*$").unwrap();

        let captures = re
            .captures(statement)
            .ok_or_else(|| format!("Malformed if statement: {}", statement))?;

        let condition = captures[1].trim().to_string();
        let label_name = captures[2].trim().to_string();

        if self.evaluate_condition(&condition)? {
            let label_index = self.labels.get(&label_name)
                .copied()
                .ok_or_else(|| format!("Label '{}' not found.", label_name))?;
            Ok(Some(label_index))
        } else {
            Ok(None)
        }
    }

    fn handle_assign(&mut self, statement: &str) -> Result<(), Box<dyn std::error::Error>> {
        let stripped = statement.trim().trim_end_matches([';', ':']);

        let eq_pos = stripped.find('=')
            .ok_or_else(|| format!("Missing '=' in assignment: {}", statement))?;

        let var_name = stripped[..eq_pos].trim().to_string();
        let rhs = stripped[eq_pos + 1..].trim().to_string();

        if var_name.is_empty() {
            return Err(format!("Empty variable name in assignment: {}", statement).into());
        }

        let arith_re = Regex::new(
            r#"^\s*(["\w.]+)\s*([+\-*/%])\s*(["\w.]+)\s*$"#
        ).unwrap();

        let new_value: String = if let Some(caps) = arith_re.captures(&rhs) {
            let lhs_tok = caps[1].trim().to_string();
            let op      = caps[2].trim().to_string();
            let rhs_tok = caps[3].trim().to_string();

            let lhs_val = self.resolve_operand(&lhs_tok)?;
            let rhs_val = self.resolve_operand(&rhs_tok)?;

            Self::apply_arithmetic(&lhs_val, &op, &rhs_val)?
        } else {
            self.resolve_operand(&rhs)?
        };

        if let Some(var) = self.vars.iter_mut().find(|v| v.name == var_name) {
            let inferred = crate::environment::Type::infer_type(&new_value);
            *var = match inferred {
                crate::environment::Type::Numeric => {
                    Self::make_numeric_var(var_name.clone(), &new_value)
                        .map_err(|e| format!("Arithmetic produced bad numeric '{}': {}", new_value, e))?
                }
                _ => Variable::from_string(var_name.clone(), &new_value),
            };
        } else {
            let inferred = crate::environment::Type::infer_type(&new_value);
            let new_var = match inferred {
                crate::environment::Type::Numeric => {
                    Self::make_numeric_var(var_name, &new_value)
                        .map_err(|e| format!("Bad numeric '{}': {}", new_value, e))?
                }
                _ => Variable::from_string(var_name, &new_value),
            };
            self.vars.push(new_var);
        }

        Ok(())
    }

    /// Constructs a `Numeric` variable from a string, choosing `from_int` when
    /// the string parses as a whole number and `from_float` otherwise.
    fn make_numeric_var(
        name: String,
        raw: &str,
    ) -> Result<Variable, Box<dyn std::error::Error>> {
        if let Ok(i) = raw.parse::<i64>() {
            Ok(Variable::from_int(name, i))
        } else {
            let f = raw.parse::<f64>()
                .map_err(|e| format!("Bad numeric '{}': {}", raw, e))?;
            Ok(Variable::from_float(name, f))
        }
    }

    fn resolve_operand(&self, token: &str) -> Result<String, Box<dyn std::error::Error>> {
        if let Some(var) = self.vars.iter().find(|v| v.name == token) {
            return Ok(var.to_string());
        }
        if token.starts_with('"') && token.ends_with('"') && token.len() >= 2 {
            return Ok(token[1..token.len() - 1].to_string());
        }
        Ok(token.to_string())
    }

    fn apply_arithmetic(
        lhs: &str,
        op: &str,
        rhs: &str,
    ) -> Result<String, Box<dyn std::error::Error>> {
        let l_num = lhs.parse::<f64>();
        let r_num = rhs.parse::<f64>();

        match (l_num, r_num) {
            (Ok(l), Ok(r)) => {
                let result = match op {
                    "+" => l + r,
                    "-" => l - r,
                    "*" => l * r,
                    "/" => {
                        if r == 0.0 {
                            return Err("Division by zero.".into());
                        }
                        l / r
                    }
                    "%" => {
                        if r == 0.0 {
                            return Err("Modulo by zero.".into());
                        }
                        l % r
                    }
                    _ => return Err(format!("Unknown arithmetic operator: {}", op).into()),
                };
                // Represent whole numbers without a decimal point so that
                // `infer_type` will later tag the result as Numeric/integer.
                if result.fract() == 0.0 && result.abs() < 1e15 {
                    Ok(format!("{}", result as i64))
                } else {
                    Ok(format!("{}", result))
                }
            }
            _ => {
                if op == "+" {
                    Ok(format!("{}{}", lhs, rhs))
                } else {
                    Err(format!(
                        "Operator '{}' cannot be applied to non-numeric operands '{}' and '{}'.",
                        op, lhs, rhs
                    ).into())
                }
            }
        }
    }

    fn evaluate_condition(&self, condition: &str) -> Result<bool, Box<dyn std::error::Error>> {
        let re = Regex::new(r#"(?i)^\s*(\w+)\s*(==|!=|>=|<=|>|<)\s*(\w+|"[^"]*"|\d+\.?\d*)\s*$"#).unwrap();

        let captures = re
            .captures(condition)
            .ok_or_else(|| format!("Unsupported condition: '{}'", condition))?;

        let lhs_name = captures[1].trim().to_string();
        let op = captures[2].trim().to_string();
        let rhs_raw = captures[3].trim().to_string();

        let lhs_var = self.vars.iter().find(|v| v.name == lhs_name)
            .ok_or_else(|| format!("Unknown variable '{}' in condition.", lhs_name))?;

        let rhs_str = if let Some(var) = self.vars.iter().find(|v| v.name == rhs_raw) {
            var.to_string()
        } else if rhs_raw.starts_with('"') && rhs_raw.ends_with('"') {
            rhs_raw[1..rhs_raw.len() - 1].to_string()
        } else {
            rhs_raw.clone()
        };

        let lhs_str = lhs_var.to_string();

        let result = if let (Ok(l), Ok(r)) = (lhs_str.parse::<f64>(), rhs_str.parse::<f64>()) {
            match op.as_str() {
                "==" => l == r,
                "!=" => l != r,
                ">"  => l > r,
                ">=" => l >= r,
                "<"  => l < r,
                "<=" => l <= r,
                _    => return Err(format!("Unknown operator: {}", op).into()),
            }
        } else {
            match op.as_str() {
                "==" => lhs_str == rhs_str,
                "!=" => lhs_str != rhs_str,
                ">"  => lhs_str > rhs_str,
                ">=" => lhs_str >= rhs_str,
                "<"  => lhs_str < rhs_str,
                "<=" => lhs_str <= rhs_str,
                _    => return Err(format!("Unknown operator: {}", op).into()),
            }
        };

        Ok(result)
    }

    fn extract_args(statement: &str) -> Result<String, Box<dyn std::error::Error>> {
        let start = statement.find('(')
            .ok_or_else(|| format!("Missing '(' in: {}", statement))?;
        let end = statement.rfind(')')
            .ok_or_else(|| format!("Missing ')' in: {}", statement))?;
        if end <= start {
            return Err(format!("Mismatched parentheses in: {}", statement).into());
        }
        Ok(statement[start + 1..end].to_string())
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
                '\\' if in_quotes => { current.push(ch); escape = true; }
                '"' => { current.push(ch); in_quotes = !in_quotes; }
                ',' if !in_quotes => {
                    let cleaned = self.clean_arg(&current);
                    if !cleaned.is_empty() { args.push(cleaned); }
                    current.clear();
                }
                _ => current.push(ch),
            }
        }

        let cleaned = self.clean_arg(&current);
        if !cleaned.is_empty() { args.push(cleaned); }
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