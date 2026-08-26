use std::fmt;

#[derive(Hash, Eq, PartialEq, Debug, Clone)]
pub enum Type {
    Numeric,
    String,
    Vector,
    Bool,
    Unknown,
    Null,
}

impl Type {
    pub fn as_str(&self) -> &'static str {
        match self {
            Type::Numeric => "numeric",
            Type::String  => "string",
            Type::Vector  => "vector",
            Type::Bool    => "bool",
            Type::Unknown => "unknown",
            Type::Null    => "null",
        }
    }
}

// Wire format for Numeric: 9 bytes.
//   byte 0 — discriminant: 0x00 = integer origin, 0x01 = float origin
//   bytes 1-8 — f64 little-endian (IEEE 754)
const DISC_INT:   u8 = 0x00;
const DISC_FLOAT: u8 = 0x01;

#[derive(Hash, Eq, PartialEq, Debug, Clone)]
pub struct Variable {
    pub var_type: Type,
    pub data: Vec<u8>,
    pub name: std::string::String,
}

#[derive(Debug, PartialEq)]
pub enum Value {
    Numeric { value: f64, is_integer: bool },
    String(std::string::String),
    Vector(Vec<std::string::String>),
    Bool(bool),
    Unknown(Vec<u8>),
    Null,
}

#[derive(Debug)]
pub enum ConvertError {
    InvalidLength { expected: usize, got: usize },
    InvalidUtf8(std::str::Utf8Error),
    InvalidBool(u8),
    InvalidDiscriminant(u8),
}

impl fmt::Display for ConvertError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::InvalidLength { expected, got } =>
                write!(f, "expected {expected} bytes, got {got}"),
            Self::InvalidUtf8(e)      => write!(f, "invalid UTF-8: {e}"),
            Self::InvalidBool(b)      => write!(f, "invalid bool byte: {b}"),
            Self::InvalidDiscriminant(d) =>
                write!(f, "invalid numeric discriminant: {d:#04x}"),
        }
    }
}

impl From<std::str::Utf8Error> for ConvertError {
    fn from(e: std::str::Utf8Error) -> Self { Self::InvalidUtf8(e) }
}

impl fmt::Display for Variable {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self.to_value() {
            Ok(Value::Numeric { value, is_integer: true })  => write!(f, "{}", value as i64),
            Ok(Value::Numeric { value, is_integer: false }) => write!(f, "{}", value),
            Ok(Value::String(s))  => write!(f, "{}", s),
            Ok(Value::Bool(b))    => write!(f, "{}", b),
            Ok(Value::Vector(v))  => write!(f, "{}", v.join(", ")),
            Ok(Value::Null)       => write!(f, "null"),
            Ok(Value::Unknown(b)) => write!(f, "{:?}", b),
            Err(e)                => write!(f, "<error: {}>", e),
        }
    }
}

impl Variable {
    /// Integer origin — stored with discriminant 0x00, value cast to f64.
    pub fn from_int(name: impl Into<std::string::String>, v: i64) -> Self {
        let mut data = vec![DISC_INT];
        data.extend_from_slice(&(v as f64).to_le_bytes());
        Self { var_type: Type::Numeric, data, name: name.into() }
    }

    /// Float origin — stored with discriminant 0x01.
    pub fn from_float(name: impl Into<std::string::String>, v: f64) -> Self {
        let mut data = vec![DISC_FLOAT];
        data.extend_from_slice(&v.to_le_bytes());
        Self { var_type: Type::Numeric, data, name: name.into() }
    }

    pub fn from_string(name: impl Into<std::string::String>, v: &str) -> Self {
        Self { var_type: Type::String, data: v.as_bytes().to_vec(), name: name.into() }
    }

    pub fn from_vector(name: impl Into<std::string::String>, v: &[&str]) -> Self {
        Self { var_type: Type::Vector, data: v.join(",").into_bytes(), name: name.into() }
    }

    pub fn from_bool(name: impl Into<std::string::String>, v: bool) -> Self {
        Self { var_type: Type::Bool, data: vec![v as u8], name: name.into() }
    }

    pub fn null(name: impl Into<std::string::String>) -> Self {
        Self { var_type: Type::Null, data: vec![], name: name.into() }
    }

    pub fn unknown(name: impl Into<std::string::String>, data: Vec<u8>) -> Self {
        Self { var_type: Type::Unknown, data, name: name.into() }
    }
}

impl Variable {
    pub fn to_value(&self) -> Result<Value, ConvertError> {
        match self.var_type {
            Type::Numeric => {
                // 9 bytes: 1 discriminant + 8 f64
                let bytes = self.expect_bytes(9)?;
                let disc = bytes[0];
                let value = f64::from_le_bytes(bytes[1..9].try_into().unwrap());
                match disc {
                    DISC_INT   => Ok(Value::Numeric { value, is_integer: true }),
                    DISC_FLOAT => Ok(Value::Numeric { value, is_integer: false }),
                    d          => Err(ConvertError::InvalidDiscriminant(d)),
                }
            }
            Type::String => {
                Ok(Value::String(std::str::from_utf8(&self.data)?.to_owned()))
            }
            Type::Vector => {
                let s = std::str::from_utf8(&self.data)?;
                Ok(Value::Vector(s.split(',').map(|p| p.to_owned()).collect()))
            }
            Type::Bool => {
                let bytes = self.expect_bytes(1)?;
                match bytes[0] {
                    0 => Ok(Value::Bool(false)),
                    1 => Ok(Value::Bool(true)),
                    b => Err(ConvertError::InvalidBool(b)),
                }
            }
            Type::Null    => Ok(Value::Null),
            Type::Unknown => Ok(Value::Unknown(self.data.clone())),
        }
    }

    fn expect_bytes(&self, n: usize) -> Result<&[u8], ConvertError> {
        if self.data.len() != n {
            return Err(ConvertError::InvalidLength { expected: n, got: self.data.len() });
        }
        Ok(&self.data)
    }
}

impl Type {
    pub fn infer_type(s: &str) -> Self {
        let trimmed = s.trim();

        if trimmed.is_empty() || trimmed.eq_ignore_ascii_case("null") {
            return Self::Null;
        }
        if trimmed.eq_ignore_ascii_case("true") || trimmed.eq_ignore_ascii_case("false") {
            return Self::Bool;
        }
        // Both integers and floats now map to Numeric.
        if trimmed.parse::<i64>().is_ok() || trimmed.parse::<f64>().is_ok() {
            return Self::Numeric;
        }
        if trimmed.contains(',') {
            return Self::Vector;
        }
        let bytes = trimmed.as_bytes();
        if bytes.len() >= 2 {
            let (first, last) = (bytes[0], bytes[bytes.len() - 1]);
            if matches!((first, last),
                (b'"',  b'"')  |
                (b'\'', b'\'') |
                (b'`',  b'`')
            ) {
                return Self::String;
            }
        }
        Self::Unknown
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn int_roundtrip() {
        let v = Variable::from_int("x", -42);
        assert_eq!(v.data.len(), 9);
        assert_eq!(
            v.to_value().unwrap(),
            Value::Numeric { value: -42.0, is_integer: true }
        );
        assert_eq!(v.to_string(), "-42");
    }

    #[test]
    fn float_roundtrip() {
        let v = Variable::from_float("pi", std::f64::consts::PI);
        assert_eq!(v.data.len(), 9);
        assert_eq!(
            v.to_value().unwrap(),
            Value::Numeric { value: std::f64::consts::PI, is_integer: false }
        );
    }

    #[test]
    fn integer_displays_without_decimal() {
        assert_eq!(Variable::from_int("n", 7).to_string(), "7");
    }

    #[test]
    fn float_displays_with_decimal() {
        let s = Variable::from_float("f", 1.5).to_string();
        assert!(s.contains('.'), "expected decimal point in '{s}'");
    }

    #[test]
    fn string_roundtrip() {
        let v = Variable::from_string("greeting", "hello");
        assert_eq!(v.to_value().unwrap(), Value::String("hello".into()));
    }

    #[test]
    fn vector_roundtrip() {
        let v = Variable::from_vector("row", &["a", "b", "c"]);
        assert_eq!(
            v.to_value().unwrap(),
            Value::Vector(vec!["a".into(), "b".into(), "c".into()])
        );
    }

    #[test]
    fn bool_roundtrip() {
        assert_eq!(Variable::from_bool("t", true).to_value().unwrap(), Value::Bool(true));
        assert_eq!(Variable::from_bool("f", false).to_value().unwrap(), Value::Bool(false));
    }

    #[test]
    fn null_roundtrip() {
        let v = Variable::null("nothing");
        assert!(v.data.is_empty());
        assert_eq!(v.to_value().unwrap(), Value::Null);
    }

    #[test]
    fn invalid_bool_byte() {
        let v = Variable { var_type: Type::Bool, data: vec![2], name: "bad".into() };
        assert!(matches!(v.to_value(), Err(ConvertError::InvalidBool(2))));
    }

    #[test]
    fn invalid_numeric_discriminant() {
        let mut data = vec![0xFF];
        data.extend_from_slice(&0f64.to_le_bytes());
        let v = Variable { var_type: Type::Numeric, data, name: "bad".into() };
        assert!(matches!(v.to_value(), Err(ConvertError::InvalidDiscriminant(0xFF))));
    }

    #[test]
    fn infer_type_numeric() {
        assert_eq!(Type::infer_type("42"),   Type::Numeric);
        assert_eq!(Type::infer_type("3.14"), Type::Numeric);
        assert_eq!(Type::infer_type("-7"),   Type::Numeric);
    }
}