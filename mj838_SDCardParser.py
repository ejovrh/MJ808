def parse_log_file_lines(input_path, header_list, divisors, decimals, signs, output_format):
    """
    @brief Parses the binary log file line by line.
    @details Reads the binary log file, parses each line into values according to the header configuration, and returns a list of parsed rows.
    @param input_path Path to the input binary log file.
    @param header_list List of header field names.
    @param divisors List of divisors for each field.
    @param decimals List of decimal places for each field.
    @param signs List of sign information for each field.
    @param output_format Output format: 'hex' or 'dec'.
    @return List of parsed rows.
    """
    rows = []  # List to store parsed rows
    skipped_log_path = 'skipped_lines.log'  # Path to the log file for skipped lines
    skipped_entries = []  # List to store skipped line entries
    with open(input_path, 'rb') as f:  # Open the input file in binary mode
        all_lines = f.read().split(b'\r\n')  # Read all lines as bytes
    total_lines = len(all_lines)  # Total number of lines
    for line_num, line in enumerate(all_lines, 1):  # Iterate through each line with line number
        if not line:  # Skip empty lines
            continue
        row = []  # List to store values for this row
        i = 0  # Index in the line
        line_len = len(line)  # Length of the line
        col = 0  # Column index
        skip_line = False  # Flag to indicate if the line should be skipped
        while i + 4 <= line_len and col < len(header_list):  # Process each 4-byte value
            value_bytes = line[i:i+4]  # Get 4 bytes for the value
            i += 4  # Move index forward
            # If not at end, expect a comma
            if i < line_len:  # If not at end of line
                if line[i:i+1] != b',':  # Check for comma separator
                    print(f"Warning: Expected comma at position {i} in line {line_num}, got {line[i:i+1]}. Skipping line.")  # Print warning
                    # Log previous, current, and next line (if available)
                    context_lines = []  # List to store context lines
                    for offset in [-1, 0, 1]:  # For previous, current, and next line
                        idx = line_num - 1 + offset  # Calculate index for context
                        if 0 <= idx < total_lines:  # Check if index is within bounds
                            raw_bytes = all_lines[idx]  # Get raw bytes for context line
                            context_lines.append((f"Line {idx+1}: ", raw_bytes))  # Store label and bytes
                    skipped_entries.append(context_lines)  # Add context to skipped entries
                    skip_line = True  # Mark to skip this line
                    break  # Stop processing this line
                i += 1  # Move past comma
            if output_format == "hex":  # If output format is hex
                row.append(f"0x{value_bytes.hex().upper()}")  # Append hex string to row
            else:
                signed = (signs[col] == 'true')  # Determine if value is signed
                val = int.from_bytes(value_bytes, byteorder='little', signed=signed)  # Convert bytes to integer
                val = val / divisors[col] if divisors[col] != 0 else val  # Divide by divisor if not zero
                val = round(val, decimals[col])  # Round to specified decimals
                row.append(val)  # Append value to row
            col += 1  # Move to next column
        if row and not skip_line:  # If row is valid and not skipped
            rows.append(row)  # Add row to rows
    # Write skipped entries to file, separated by -------
    if skipped_entries:  # If there are skipped entries
        with open(skipped_log_path, 'wb') as logf:  # Open log file in binary mode
            for idx, entry in enumerate(skipped_entries):  # For each skipped entry
                for label, raw_bytes in entry:  # For each context line
                    logf.write(label.encode('utf-8'))  # Write label to log
                    logf.write(raw_bytes)  # Write raw bytes to log
                    logf.write(b'\n')  # Write newline
                if idx < len(skipped_entries) - 1:  # If not the last skipped entry
                    logf.write(b'-------\n')  # Write separator
    return rows  # Return parsed rows


# Path to the mj838.h header file (update this path as needed)
header_path = 'mj8x8/Core/Inc/mj838/mj838.h'  # Path to the header file

import re  # Import the regular expressions module

"""
@file mj838_SDCardParser.py
@brief Parses binary SD card logs and exports them to Excel.
@details This script reads a binary log file, parses its contents according to field definitions in mj838.h, and exports the parsed data to an Excel file. It supports both hexadecimal and decimal output formats.
"""

def parse_header_config(header_path):  # Function to parse the mj838.h file for CSV header definition
    """
    @brief Parses the mj838.h file for CSV header definition.
    @details Extracts field names, divisors, decimal places, and sign information from the mj838.h header file. Used to interpret the binary log data fields.
    @param header_path Path to the mj838.h header file.
    @return Tuple of (header_list, divisors, decimals, signs).
    """
    """
    Parses the mj838.h file for CSV header definition, extracting field names, divisors, and decimal places.
    Returns (header_list, divisors, decimals)
    """
    with open(header_path, 'r', encoding='utf-8') as f:  # Open the header file for reading
        lines = f.readlines()  # Read all lines from the file
    in_enum = False  # Flag to indicate if inside the enum definition
    header_list = []  # List to store header field names
    divisors = []  # List to store divisors for each field
    decimals = []  # List to store decimal places for each field
    signs = []  # List to store sign information for each field
    for line in lines:  # Iterate through each line in the file
        if '// CSV header definition' in line:  # Look for the start of the CSV header definition
            if not in_enum:  # If not already inside the enum
                in_enum = True  # Set flag to True
                continue  # Skip to next line
            else:
                break  # If already inside, break out of the loop
        if in_enum:  # If inside the enum definition
            # Stop at LOG_FIELD_COUNT
            if 'LOG_FIELD_COUNT' in line:  # End of relevant enum section
                break
            # Match LOG_* lines with descr, div, dec
            # Only consider up to the first semicolon
            comment = line.split(';', 1)[0]  # Only consider code before the first semicolon
            m = re.match(r'\s*LOG_\w+\s*=\s*\d+,\s*//\s*descr=([^,]+),div=([^,]+),dec=([0-9]+)(?:,sign=([^,]+))?', comment)  # Regex to extract field info
            if m:  # If regex matches
                descr = m.group(1).strip()  # Extract description
                try:
                    div = int(m.group(2).strip())  # Extract divisor
                except Exception:
                    print(f"Warning: Could not parse divisor in line: {line.strip()}")  # Print warning if divisor can't be parsed
                    continue  # Skip this line
                dec = int(m.group(3).strip())  # Extract decimal places
                sign = m.group(4).strip().lower() if m.group(4) else 'false'  # Extract sign info
                header_list.append(descr)  # Add description to list
                divisors.append(div)  # Add divisor to list
                decimals.append(dec)  # Add decimals to list
                signs.append(sign)  # Add sign info to list
    return header_list, divisors, decimals, signs  # Return all extracted lists




import sys  # Import the sys module for command-line arguments
import os  # Import the os module for file and path operations
def parse_log_file_lines(input_path, header_list, divisors, decimals, signs, output_format):
    """
    @brief Parses the binary log file line by line.
    @details Reads the binary log file, parses each line into values according to the header configuration, and returns a list of parsed rows.
    @param input_path Path to the input binary log file.
    @param header_list List of header field names.
    @param divisors List of divisors for each field.
    @param decimals List of decimal places for each field.
    @param signs List of sign information for each field.
    @param output_format Output format: 'hex' or 'dec'.
    @return List of parsed rows.
    """
    skipped_log_path = 'skipped_lines.log'  # Path to the log file for skipped lines
    skipped_entries = []  # List to store skipped line entries
    with open(input_path, 'rb') as f:  # Open the input file in binary mode
        all_lines = f.read().split(b'\r\n')  # Read all lines as bytes
    total_lines = len(all_lines)  # Total number of lines
    rows = []  # List to store parsed rows
    for line_num, line in enumerate(all_lines, 1):  # Iterate through each line with line number
        if not line:  # Skip empty lines
            continue
        row = []  # List to store values for this row
        i = 0  # Index in the line
        line_len = len(line)  # Length of the line
        col = 0  # Column index
        skip_line = False  # Flag to indicate if the line should be skipped
        while i + 4 <= line_len and col < len(header_list):  # Process each 4-byte value
            value_bytes = line[i:i+4]  # Get 4 bytes for the value
            i += 4  # Move index forward
            # If not at end, expect a comma
            if i < line_len:  # If not at end of line
                if line[i:i+1] != b',':  # Check for comma separator
                    print(f"Warning: Expected comma at position {i} in line {line_num}, got {line[i:i+1]}. Skipping line.")  # Print warning
                    # Log previous, current, and next line (if available)
                    context_lines = []  # List to store context lines
                    for offset in [-1, 0, 1]:  # Previous, current, next
                        idx = line_num - 1 + offset  # Calculate index
                        if 0 <= idx < total_lines:  # Check bounds
                            raw_bytes = all_lines[idx]  # Get raw bytes
                            context_lines.append((f"Line {idx+1}: ", raw_bytes))  # Store label and bytes
                    skipped_entries.append(context_lines)  # Add to skipped entries
                    skip_line = True  # Mark to skip this line
                    break  # Stop processing this line
                i += 1  # Move past comma
            if output_format == "hex":  # If output format is hex
                row.append(f"0x{value_bytes.hex().upper()}")  # Append hex string
            else:
                signed = (signs[col] == 'true')  # Determine if value is signed
                val = int.from_bytes(value_bytes, byteorder='little', signed=signed)  # Convert bytes to integer
                # Divide by divisor and round to decimal places
                val = val / divisors[col] if divisors[col] != 0 else val  # Divide by divisor if not zero
                val = round(val, decimals[col])  # Round to specified decimals
                row.append(val)  # Append value to row
            col += 1  # Move to next column
        if row and not skip_line:  # If row is valid and not skipped
            rows.append(row)  # Add row to rows
    # Write skipped entries to file, separated by -------
    if skipped_entries:  # If there are skipped entries
        with open(skipped_log_path, 'wb') as logf:  # Open log file in binary mode
            for idx, entry in enumerate(skipped_entries):  # For each skipped entry
                for label, raw_bytes in entry:  # For each context line
                    logf.write(label.encode('utf-8'))  # Write label
                    logf.write(raw_bytes)  # Write raw bytes
                    logf.write(b'\n')  # Newline
                if idx < len(skipped_entries) - 1:  # Separator between entries
                    logf.write(b'-------\n')  # Separator
    return rows  # Return parsed rows


def main():  # Main function
    """
    @brief Main entry point for the SD card parser script.
    @details Handles command-line arguments, parses the input binary file, processes the data according to the header configuration, and exports the results to an Excel file. Supports both hexadecimal and decimal output formats.
    """
    if len(sys.argv) != 4:  # Check for correct number of command-line arguments
        print("Usage: python mj838_SDCardParser.py <full_path_to_input_file> <hex|dec> <sequences_to_keep>")
        print("  <sequences_to_keep>: 0=all, 1=last, 2=last two, etc.")
        sys.exit(1)
    input_path = sys.argv[1]  # Get input file path from arguments
    output_format = sys.argv[2].lower()  # Get output format (hex or dec)
    try:
        sequences_to_keep = int(sys.argv[3])  # Get number of sequences to keep
    except ValueError:
        print("Third argument <sequences_to_keep> must be an integer.")
        sys.exit(1)
    if output_format not in ("hex", "dec"):  # Validate output format
        print("Second argument must be 'hex' or 'dec'.")
        sys.exit(1)
    if not os.path.isfile(input_path):  # Check if input file exists
        print(f"Error: File '{input_path}' does not exist.")
        sys.exit(1)

    print(f"Parsing file: {input_path}")
    # Parse header config from mj838.h
    header_list, divisors, decimals, signs = parse_header_config(header_path)

    # Parse the log file lines using the new function
    rows = parse_log_file_lines(input_path, header_list, divisors, decimals, signs, output_format)

    # Filter rows by sequence if needed
    scroll_to_row = 0  # Default: top of sheet
    reset_indices = [0]
    if rows:
        # Find indices where the first column resets (decreases)
        for i in range(1, len(rows)):
            if rows[i][0] < rows[i-1][0]:
                reset_indices.append(i)
        reset_indices.append(len(rows))
        # Each segment is rows[reset_indices[j]:reset_indices[j+1]]
        segments = [rows[reset_indices[j]:reset_indices[j+1]] for j in range(len(reset_indices)-1)]
        if sequences_to_keep != 0:
            # Keep only the last N segments
            rows = [row for segment in segments[-sequences_to_keep:] for row in segment]
        # Always scroll to the start of the last sequence
        scroll_to_row = reset_indices[-2] + 2  # +2 for header and 1-based Excel row

    print(f"Lines parsed: {len(rows)}")
    if output_format == "hex":
        print("Note: Excel output contains raw hex values (ASCII, prefixed with '0x').")
    else:
        print("Note: Excel output contains decimal values.")
    try:
        import pandas as pd
    except ImportError:
        print("pandas is required to export to Excel. Please install it with 'pip install pandas'.")
        sys.exit(1)
    import datetime
    script_dir = os.path.dirname(os.path.abspath(__file__))
    timestamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
    output_excel = os.path.join(script_dir, f"mj838_SDCard_{timestamp}.xlsx")
    df = pd.DataFrame(rows, columns=header_list)
    with pd.ExcelWriter(output_excel, engine='openpyxl') as writer:
        df.to_excel(writer, index=False, header=True)
        workbook = writer.book
        worksheet = writer.sheets['Sheet1']
        # Set number format for all cells (decimal mode)
        if output_format == "dec":
            for col_idx, dec in enumerate(decimals, 1):
                fmt = f"0.{''.join(['0']*dec)}" if dec > 0 else "0"
                for cell in worksheet.iter_cols(min_col=col_idx, max_col=col_idx, min_row=2):
                    for c in cell:
                        c.number_format = fmt
        # Auto-fit column widths to display full text
        for col in worksheet.columns:
            max_length = 0
            col_letter = col[0].column_letter
            for cell in col:
                try:
                    cell_value = str(cell.value) if cell.value is not None else ""
                    if len(cell_value) > max_length:
                        max_length = len(cell_value)
                except Exception:
                    pass
            adjusted_width = max_length + 2
            worksheet.column_dimensions[col_letter].width = adjusted_width
        # Scroll to the start of the last sequence if only the last is kept
        if scroll_to_row > 0:
            worksheet.sheet_view.topLeftCell = f"A{scroll_to_row}"
    print(f"Exported to Excel: {output_excel}")
    # Automatically open the exported Excel file (Windows only)
    try:
        import subprocess
        subprocess.Popen(['start', '', output_excel], shell=True)
    except Exception as e:
        print(f"Could not open Excel file automatically: {e}")

if __name__ == "__main__":  # If script is run directly
    # @brief Script entry point.
    # @details Calls the main() function if the script is executed directly.
    main()  # Call main function

