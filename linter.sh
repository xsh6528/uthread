#!/bin/bash

cpplint --filter=-legal/copyright,-build/include,-build/c++11,-runtime/int,-runtime/references,-readability/casting,-readability/namespace \
        --repository=. \
        --root=include \
        --extensions=hpp,cpp,h,c \
        --recursive \
        ./bench ./examples ./src ./tests ./include

echo "Done!"
