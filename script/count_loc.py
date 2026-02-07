import os

def count_stats(target_dir):
    h_count = 0
    cpp_count = 0
    total_lines = 0
    
    if not os.path.exists(target_dir):
        return None

    for root, dirs, files in os.walk(target_dir):
        for file in files:
            if file.endswith('.h') or file.endswith('.cpp'):
                if file.endswith('.h'):
                    h_count += 1
                else:
                    cpp_count += 1
                
                file_path = os.path.join(root, file)
                try:
                    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                        total_lines += sum(1 for _ in f)
                except Exception as e:
                    print(f"Error reading {file_path}: {e}")
                    
    return h_count, cpp_count, total_lines

def main():
    base_src = "src"
    sub_dirs = ["engine_runtime", "engine_test", "engine_tool"]
    
    print(f"{'Directory':<20} | {'h files':<8} | {'cpp files':<10} | {'Total Lines':<12}")
    print("-" * 60)
    
    grand_total_h = 0
    grand_total_cpp = 0
    grand_total_lines = 0
    
    for sub in sub_dirs:
        path = os.path.join(base_src, sub)
        stats = count_stats(path)
        
        if stats:
            h, cpp, lines = stats
            print(f"{sub:<20} | {h:<8} | {cpp:<10} | {lines:<12}")
            grand_total_h += h
            grand_total_cpp += cpp
            grand_total_lines += lines
        else:
            print(f"{sub:<20} | Not Found")
            
    print("-" * 60)
    print(f"{'Total':<20} | {grand_total_h:<8} | {grand_total_cpp:<10} | {grand_total_lines:<12}")

if __name__ == "__main__":
    main()
