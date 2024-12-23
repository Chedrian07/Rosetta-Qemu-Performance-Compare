#------------------------------------------------------------
# 컴파일러/옵션 설정
#------------------------------------------------------------
CC ?= gcc
CFLAGS = -O2 -Wall
LDFLAGS =
MATHLIB = -lm
PTHREADLIB = -pthread

SRC_DIR = src
OUT_DIR = out

#------------------------------------------------------------
# 빌드할 바이너리 구분
#   1) math 라이브러리 필요한 소스
#   2) pthread 라이브러리 필요한 소스
#   3) math 라이브러리와 pthread 라이브러리 필요 없는 소스
#------------------------------------------------------------
MATH_TARGETS = arith_benchmark
PTHREAD_TARGETS = multi_threaded_computation
REG_TARGETS = array_sorting        \
             file_io_benchmark    \
             hashing_benchmark    \
             matrix_multiplication \
             memory_allocation    \
             parallel_computation \
             prime_numbers        \
             recursive_fibonacci  

ALL_TARGETS = $(MATH_TARGETS) $(PTHREAD_TARGETS) $(REG_TARGETS)

#------------------------------------------------------------
# 기본 타겟: out 디렉토리 생성 후 모든 타겟 빌드
#------------------------------------------------------------
all: $(OUT_DIR) \
     $(addprefix $(OUT_DIR)/,$(ALL_TARGETS))

# out 디렉토리 생성
$(OUT_DIR):
	mkdir -p $(OUT_DIR)

#------------------------------------------------------------
# math 라이브러리가 필요한 타겟 (예: arith_benchmark)
#   -lm 옵션 추가
#------------------------------------------------------------
$(OUT_DIR)/arith_benchmark: $(SRC_DIR)/arith_benchmark.c
	$(CC) $(CFLAGS) $< -o $@ $(MATHLIB)

#------------------------------------------------------------
# pthread 라이브러리가 필요한 타겟 (예: multi_threaded_computation)
#   -pthread 옵션 추가
#------------------------------------------------------------
$(OUT_DIR)/multi_threaded_computation: $(SRC_DIR)/multi_threaded_computation.c
	$(CC) $(CFLAGS) $< -o $@ $(PTHREADLIB)

#------------------------------------------------------------
# math 라이브러리와 pthread 라이브러리가 필요 없는 일반 타겟
#   pattern rule을 이용해 처리
#------------------------------------------------------------
$(OUT_DIR)/%: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

#------------------------------------------------------------
# 클린업
#------------------------------------------------------------
clean:
	rm -rf $(OUT_DIR)/*

#------------------------------------------------------------
# 헬프 메시지
#------------------------------------------------------------
help:
	@echo "사용 가능한 명령:"
	@echo "  make          - 모든 바이너리를 컴파일하여 '$(OUT_DIR)'에 저장"
	@echo "  make clean    - '$(OUT_DIR)' 디렉토리 및 바이너리 삭제"
	@echo "  make help     - 사용 가능한 명령어 출력"