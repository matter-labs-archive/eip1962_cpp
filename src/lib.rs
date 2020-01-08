mod api;
mod enums;

pub use self::enums::*;

const MAX_OUTPUT_LEN: usize = 256*3*2; // output if Fp = 1023 bits, Fp3 extension, affine point
const ERROR_DESCRIPTION_LEN: usize = 256;

pub fn run(bytes: &[u8]) -> Result<Vec<u8>, String> {
    let mut result = vec![0u8; MAX_OUTPUT_LEN];
    let mut error_description_buffer = vec![0u8; ERROR_DESCRIPTION_LEN];
    let input = bytes.as_ptr() as *const std::os::raw::c_char;
    let output = result.as_mut_ptr() as *mut std::os::raw::c_char;
    let error_buffer = error_description_buffer.as_mut_ptr() as *mut std::os::raw::c_char;
    let input_len = bytes.len() as u32;
    let mut output_len = 0u32;
    let mut error_description_len = 0u32;

    let success = unsafe { self::api::run(
        input, 
        input_len, 
        output, 
        &mut output_len as *mut u32,
        error_buffer,
        &mut error_description_len as *mut u32
    ) };
    if success == 0 {
        if error_description_len == 0 {
            return Err("C++ api returned empty error description".to_string());
        }
        error_description_buffer.truncate(error_description_len as usize);
        let error_description_string = std::ffi::CString::new(error_description_buffer);
        match error_description_string {
            Ok(c_string) => {
                let string = c_string.into_string();
                match string {
                    Ok(string) => {
                        return Err(string);
                    },
                    Err(err) => {
                        return Err(format!("Error on conversion of string description, {:?}", err));
                    }
                }
            },
            Err(n_error) => {
                return Err(format!("CString containts empty bytes in a middle, {:?}", n_error));
            }
        }
    }

    result.truncate(output_len as usize);

    Ok(result)
}

pub fn meter(bytes: &[u8]) -> Result<u64, String> {
    let input = bytes.as_ptr() as *const std::os::raw::c_char;
    let input_len = bytes.len() as u32;
    let mut gas = 0u64;

    let success = unsafe { self::api::meter_gas(
        input, 
        input_len, 
        &mut gas as *mut u64
    ) };
    if success == 0 {
        return Err("Failed to meter gas".to_string());
    }

    Ok(gas)
}

// This is pure rust API
pub fn perform_operation(operation: OperationType, bytes: &[u8]) -> Result<Vec<u8>, String> {
    let raw_operation_value: std::os::raw::c_char = unsafe { std::mem::transmute(operation.as_u8()) };

    let mut result = vec![0u8; MAX_OUTPUT_LEN];
    let mut error_description_buffer = vec![0u8; ERROR_DESCRIPTION_LEN];
    let input = bytes.as_ptr() as *const std::os::raw::c_char;
    let output = result.as_mut_ptr() as *mut std::os::raw::c_char;
    let error_buffer = error_description_buffer.as_mut_ptr() as *mut std::os::raw::c_char;
    let input_len = bytes.len() as u32;
    let mut output_len = 0u32;
    let mut error_description_len = 0u32;

    let is_error = unsafe { self::api::c_perform_operation(
        raw_operation_value,
        input, 
        input_len, 
        output, 
        &mut output_len as *mut u32,
        error_buffer,
        &mut error_description_len as *mut u32
    ) };
    if is_error != 0 {
        if error_description_len == 0 {
            return Err("C++ api returned empty error description".to_string());
        }
        error_description_buffer.truncate(error_description_len as usize);
        let error_description_string = std::ffi::CString::new(error_description_buffer);
        match error_description_string {
            Ok(c_string) => {
                let string = c_string.into_string();
                match string {
                    Ok(string) => {
                        return Err(string);
                    },
                    Err(err) => {
                        return Err(format!("Error on conversion of string description, {:?}", err));
                    }
                }
            },
            Err(n_error) => {
                return Err(format!("CString containts empty bytes in a middle, {:?}", n_error));
            }
        }
    }

    result.truncate(output_len as usize);

    Ok(result)
}

pub fn meter_operation(operation: OperationType, bytes: &[u8]) -> Result<u64, String> {
    let raw_operation_value: std::os::raw::c_char = unsafe { std::mem::transmute(operation.as_u8()) };

    let mut result = vec![0u8; MAX_OUTPUT_LEN];
    let mut error_description_buffer = vec![0u8; ERROR_DESCRIPTION_LEN];
    let input = bytes.as_ptr() as *const std::os::raw::c_char;
    let error_buffer = error_description_buffer.as_mut_ptr() as *mut std::os::raw::c_char;

    let input_len = bytes.len() as u32;
    let mut gas = 0u64;
    let mut error_description_len = 0u32;

    let is_error = unsafe { self::api::c_meter_operation(
        raw_operation_value,
        input, 
        input_len, 
        &mut gas as *mut u64,
        error_buffer,
        &mut error_description_len as *mut u32
    ) };
    if is_error != 0 {
        if error_description_len == 0 {
            return Err("C++ api returned empty error description".to_string());
        }
        error_description_buffer.truncate(error_description_len as usize);
        let error_description_string = std::ffi::CString::new(error_description_buffer);
        match error_description_string {
            Ok(c_string) => {
                let string = c_string.into_string();
                match string {
                    Ok(string) => {
                        return Err(string);
                    },
                    Err(err) => {
                        return Err(format!("Error on conversion of string description, {:?}", err));
                    }
                }
            },
            Err(n_error) => {
                return Err(format!("CString containts empty bytes in a middle, {:?}", n_error));
            }
        }
    }

    Ok(gas)
}