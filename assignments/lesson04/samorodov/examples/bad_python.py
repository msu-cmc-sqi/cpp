def calculate_average(numbers):
    total = 0
    for i in range(len(numbers)):
        total += numbers[i]
    return total / len(numbers)  # Деление на ноль при пустом списке

def process_data(data):
    x = None
    if data:
        x = data[0]
    return x.upper()  # Возможный AttributeError

# Необработанные случаи
result1 = calculate_average([])  # Вызовет ZeroDivisionError
result2 = process_data([])       # Вызовет AttributeError

print("Results:", result1, result2)