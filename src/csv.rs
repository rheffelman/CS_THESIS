use std::fs;

#[derive(Debug, Clone, PartialEq)]
pub enum ColumnType {
    Empty,
    Boolean,
    Integer,
    Float,
    String,
}

pub struct CSV {
    pub fp: String,
    pub delim: String,
    pub row_length: u32,
    pub num_rows: u32,
    pub has_header: bool,
    pub headers: Option<Vec<String>>,
    pub data: Vec<String>,
    pub column_types: Vec<ColumnType>,
}

impl CSV {
    pub fn new(fp: String, delim: Option<String>, has_header: bool) -> CSV {
        let actual_delim = delim.unwrap_or(",".to_string());
        let contents = fs::read_to_string(&fp).unwrap();

        let mut data = Vec::new();
        let mut num_rows = 0u32;
        let mut row_length = 0u32;
        let mut headers: Option<Vec<String>> = None;

        for (i, line) in contents.lines().enumerate() {
            let fields: Vec<String> = line
                .split(&actual_delim)
                .map(|s| s.trim().to_string())
                .collect();

            if i == 0 {
                row_length = fields.len() as u32;

                if has_header {
                    headers = Some(fields);
                    continue;
                }
            }

            data.extend(fields);
            num_rows += 1;
        }

        let mut csv = CSV {
            fp,
            delim: actual_delim,
            row_length,
            num_rows,
            has_header,
            headers,
            data,
            column_types: Vec::new(),
        };

        csv.infer_column_types();
        csv
    }

    fn classify_value(value: &str) -> ColumnType {
        let trimmed = value.trim();

        if trimmed.is_empty() {
            return ColumnType::Empty;
        }

        let lower = trimmed.to_ascii_lowercase();

        if matches!(lower.as_str(), "true" | "false" | "yes" | "no" | "t" | "f") {
            return ColumnType::Boolean;
        }

        if trimmed.parse::<i64>().is_ok() {
            return ColumnType::Integer;
        }

        if trimmed.parse::<f64>().is_ok() {
            return ColumnType::Float;
        }

        ColumnType::String
    }

    fn merge_types(left: ColumnType, right: ColumnType) -> ColumnType {
        use ColumnType::*;

        match (left, right) {
            (Empty, t) => t,
            (t, Empty) => t,

            (Boolean, Boolean) => Boolean,
            (Integer, Integer) => Integer,
            (Float, Float) => Float,
            (String, String) => String,

            (Integer, Float) | (Float, Integer) => Float,

            (Boolean, Integer)
            | (Integer, Boolean)
            | (Boolean, Float)
            | (Float, Boolean)
            | (Boolean, String)
            | (String, Boolean)
            | (Integer, String)
            | (String, Integer)
            | (Float, String)
            | (String, Float) => String,
        }
    }

    pub fn infer_column_types(&mut self) {
        let cols = self.row_length as usize;

        if cols == 0 {
            self.column_types.clear();
            return;
        }

        let mut inferred = vec![ColumnType::Empty; cols];

        for row in 0..self.num_rows as usize {
            for col in 0..cols {
                let index = row * cols + col;

                if index >= self.data.len() {
                    continue;
                }

                let cell_type = CSV::classify_value(&self.data[index]);
                inferred[col] = CSV::merge_types(inferred[col].clone(), cell_type);
            }
        }

        self.column_types = inferred;
    }

    pub fn print_csv(&self) {
        println!("Filepath   : {}", self.fp);
        println!("Delimiter  : {:?}", self.delim);
        println!("Rows       : {}", self.num_rows);
        println!("Row length : {}", self.row_length);
        println!("Has header : {}", self.has_header);
        println!("Cells      : {}", self.data.len());
        println!("Col types  : {:?}", self.column_types);
        println!("\n");

        if self.row_length == 0 {
            println!("[empty csv]");
            return;
        }

        let cols = self.row_length as usize;
        let mut col_widths = vec![0usize; cols];

        if let Some(headers) = &self.headers {
            for col in 0..cols {
                if col < headers.len() {
                    col_widths[col] = headers[col].len();
                }
            }
        }

        for row in 0..self.num_rows as usize {
            for col in 0..cols {
                let index = row * cols + col;

                if index < self.data.len() {
                    let cell_len = self.data[index].len();
                    if cell_len > col_widths[col] {
                        col_widths[col] = cell_len;
                    }
                }
            }
        }

        if let Some(headers) = &self.headers {
            for col in 0..cols {
                if col < headers.len() {
                    print!("{:<width$}", headers[col], width = col_widths[col]);
                } else {
                    print!("{:<width$}", "", width = col_widths[col]);
                }

                if col < cols - 1 {
                    print!(" {} ", self.delim);
                }
            }
            println!();

            for col in 0..cols {
                print!("{:-<width$}", "", width = col_widths[col]);
                if col < cols - 1 {
                    print!("-+-");
                }
            }
            println!();
        }

        for row in 0..self.num_rows as usize {
            for col in 0..cols {
                let index = row * cols + col;

                if index < self.data.len() {
                    print!("{:<width$}", self.data[index], width = col_widths[col]);
                } else {
                    print!("{:<width$}", "", width = col_widths[col]);
                }

                if col < cols - 1 {
                    print!(" {} ", self.delim);
                }
            }
            println!();
        }
    }
}