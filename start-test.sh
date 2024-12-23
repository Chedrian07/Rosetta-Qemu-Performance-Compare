#!/bin/bash

##############################################################################
# 색상 설정
##############################################################################
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

##############################################################################
# 0. 인자 검증
##############################################################################
if [[ $# -ne 1 ]]; then
    echo -e "${RED}[ERROR] 인자가 필요합니다.${NC}"
    echo -e "사용법: $0 [rosetta|qemu|native]"
    exit 1
fi

ENVIRONMENT=$1

if [[ "$ENVIRONMENT" != "rosetta" && "$ENVIRONMENT" != "qemu" && "$ENVIRONMENT" != "native" ]]; then
    echo -e "${RED}[ERROR] 잘못된 인자입니다. 'rosetta', 'qemu', 또는 'native'를 입력해주세요.${NC}"
    echo -e "사용법: $0 [rosetta|qemu|native]"
    exit 1
fi

# 로그 디렉토리 설정
if [[ "$ENVIRONMENT" == "rosetta" ]]; then
    LOG_BASE_DIR="rosetta_log"
    CROSS_COMPILE_PREFIX="x86_64-linux-gnu-"
elif [[ "$ENVIRONMENT" == "qemu" ]]; then
    LOG_BASE_DIR="qemu_log"
    CROSS_COMPILE_PREFIX="x86_64-linux-gnu-"
elif [[ "$ENVIRONMENT" == "native" ]]; then
    LOG_BASE_DIR="native_log"
    CROSS_COMPILE_PREFIX=""
fi

##############################################################################
# 1. 빌드 및 준비 단계
##############################################################################
echo -e "${GREEN}[INFO] Building all binaries...${NC}"
if [[ -z "$CROSS_COMPILE_PREFIX" ]]; then
    make
else
    make CC="${CROSS_COMPILE_PREFIX}gcc"
fi

if [[ $? -ne 0 ]]; then
    echo -e "${RED}[ERROR] Make failed. Exiting...${NC}"
    exit 1
fi

# 로그 기본 디렉토리 생성
mkdir -p "$LOG_BASE_DIR"

# 테스트할 바이너리 목록 (out 디렉토리에 생성된 실행 파일 이름과 동일)
# string_manipulation 제거됨
binaries=(
  "arith_benchmark"
  "array_sorting"
  "matrix_multiplication"
  "file_io_benchmark"
  "prime_numbers"
  "recursive_fibonacci"
  "hashing_benchmark"
  "memory_allocation"
  "parallel_computation"
  "multi_threaded_computation"
)

echo -e "${BLUE}[INFO] Starting benchmark tests for '${ENVIRONMENT}' environment...${NC}"

##############################################################################
# 2. 순차적 벤치마크 실행
##############################################################################
for bin in "${binaries[@]}"; do
    # 확인: 바이너리 파일이 존재하는지 (혹은 실행 가능 상태인지)
    if [[ ! -f "./out/$bin" ]]; then
      echo -e "${RED}[ERROR] Binary './out/$bin' not found. Skipping...${NC}"
      continue
    fi

    # 바이너리별 로그 디렉토리 생성
    mkdir -p "$LOG_BASE_DIR/${bin}"
    
    # 바이너리 실행 명령어 설정
    if [[ "$ENVIRONMENT" == "qemu" ]]; then
        # qemu-x86_64-static이 설치되어 있는지 확인
        if ! command -v qemu-x86_64-static &> /dev/null; then
            echo -e "${RED}[ERROR] 'qemu-x86_64-static' 명령어를 찾을 수 없습니다. 'qemu-user-static' 패키지가 설치되었는지 확인해주세요.${NC}"
            exit 1
        fi
        EXEC_CMD="./out/$bin" #qemu-x86_64-static ./out/$bin"
    else
        EXEC_CMD="./out/$bin"
    fi

    #########################################
    # 2-0) sar 시작 (CPU info만 수집)
    #########################################
    echo -e "${CYAN}[SAR] Starting system performance monitoring for '$bin'...${NC}"
    sar -u 1 100 -o "$LOG_BASE_DIR/${bin}/sar_data.bin" >/dev/null 2>&1 &
    SAR_PID=$!
    
    #########################################
    # 2-1) dstat 시작
    #########################################
    echo -e "${CYAN}[DSTAT] Starting system performance monitoring for '$bin'...${NC}"
    dstat -cdngym 1 100 --output "$LOG_BASE_DIR/${bin}/dstat.csv" >/dev/null 2>&1 &
    DSTAT_PID=$!
    
    #########################################
    # 2-2) time 벤치마크
    #########################################
    echo -e "${CYAN}[TIME] Running $bin...${NC}"
    /usr/bin/time -v $EXEC_CMD > "$LOG_BASE_DIR/${bin}/time.log" 2>&1
    TIME_EXIT_STATUS=$?
    if [[ $TIME_EXIT_STATUS -ne 0 ]]; then
        echo -e "${YELLOW}[WARNING] Time benchmark for '$bin' completed with warnings or errors (Exit Status: $TIME_EXIT_STATUS).${NC}"
    fi

    #########################################
    # 2-3) sar 종료 및 텍스트 변환
    #########################################
    echo -e "${CYAN}[SAR] Stopping system performance monitoring for '$bin'...${NC}"
    kill $SAR_PID
    wait $SAR_PID 2>/dev/null
    sar -f "$LOG_BASE_DIR/${bin}/sar_data.bin" > "$LOG_BASE_DIR/${bin}/sar.log" 2>/dev/null

    #########################################
    # 2-4) dstat 종료
    #########################################
    echo -e "${CYAN}[DSTAT] Stopping system performance monitoring for '$bin'...${NC}"
    kill $DSTAT_PID
    wait $DSTAT_PID 2>/dev/null
    
    echo -e "${GREEN}[INFO] Finished all tests for '${bin}'.${NC}"
    echo -e "${NC}---------------------------------------------${NC}"
done

##############################################################################
# 3. FIO 벤치마크 실행 (한 번만)
##############################################################################
echo -e "${YELLOW}[FIO] Starting global I/O benchmark (100MB read/write)...${NC}"

FIO_TEST_FILE="$LOG_BASE_DIR/global_fio_test_file"

fio --randrepeat=1 \
    --ioengine=libaio \
    --direct=1 \
    --gtod_reduce=1 \
    --name="global_fio_test" \
    --filename="$FIO_TEST_FILE" \
    --bs=4k \
    --iodepth=64 \
    --size=100M \
    --readwrite=randrw \
    --rwmixread=50 \
    --runtime=10 \
    --output="$LOG_BASE_DIR/fio.log"

FIO_EXIT_STATUS=$?
if [[ $FIO_EXIT_STATUS -ne 0 ]]; then
    echo -e "${YELLOW}[WARNING] FIO benchmark completed with warnings or errors (Exit Status: $FIO_EXIT_STATUS).${NC}"
else
    echo -e "${GREEN}[INFO] Global FIO benchmark completed.${NC}"
fi
echo -e "${NC}---------------------------------------------${NC}"

##############################################################################
# 4. sysbench 벤치마크 실행 (한 번만)
##############################################################################
echo -e "${YELLOW}[SYSBENCH] Starting CPU benchmark...${NC}"
sysbench cpu --cpu-max-prime=20000 run > "$LOG_BASE_DIR/sysbench_cpu.log" 2>&1
SYSBENCH_EXIT_STATUS=$?
if [[ $SYSBENCH_EXIT_STATUS -ne 0 ]]; then
    echo -e "${YELLOW}[WARNING] Sysbench CPU benchmark completed with warnings or errors (Exit Status: $SYSBENCH_EXIT_STATUS).${NC}"
else
    echo -e "${GREEN}[INFO] SYSBENCH CPU benchmark completed.${NC}"
fi
echo -e "${NC}---------------------------------------------${NC}"

##############################################################################
# 5. 정리 (필요 시)
##############################################################################
rm -rf "$LOG_BASE_DIR"/*/*_test.0.0
rm -rf "$LOG_BASE_DIR"/*/*.bin
rm -rf *.bin
rm -rf *.fio
make clean

##############################################################################
# 6. 완료 메시지
##############################################################################
echo -e "${BLUE}[INFO] All benchmarks completed. Logs are in the '${LOG_BASE_DIR}/' directory.${NC}"