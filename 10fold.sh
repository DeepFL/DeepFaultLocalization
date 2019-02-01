  #! /bin/bash
python main.py . ./result/CrossValidation 10fold 1 fc CrossValidation softmax 81 1 1 &
python main.py . ./result/CrossValidation 10fold 2 fc CrossValidation softmax 81 1 2 &
python main.py . ./result/CrossValidation 10fold 3 fc CrossValidation softmax 81 1 3 &
python main.py . ./result/CrossValidation 10fold 4 fc CrossValidation softmax 81 1 3 &
python main.py . ./result/CrossValidation 10fold 5 fc CrossValidation softmax 81 1 4 &
python main.py . ./result/CrossValidation 10fold 6 fc CrossValidation softmax 81 1 5 &
python main.py . ./result/CrossValidation 10fold 7 fc CrossValidation softmax 81 1 5 &
python main.py . ./result/CrossValidation 10fold 8 fc CrossValidation softmax 81 1 6 &
python main.py . ./result/CrossValidation 10fold 9 fc CrossValidation softmax 81 1 6 &
python main.py . ./result/CrossValidation 10fold 10 fc CrossValidation softmax 81 1 7 &
