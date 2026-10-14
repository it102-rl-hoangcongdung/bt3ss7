#include <stdio.h>

// Khai báo struct chưa tối ưu (dễ phát sinh padding bytes)
struct DeviceUnoptimized {
    char status;       // 1 byte
    double voltage;    // 8 bytes (phát sinh 7 byte đệm trước trường này)
    int active_time;   // 4 bytes
    float current;     // 4 bytes
    float power;       // 4 bytes
};

// Khai báo struct đã tối ưu (Flat Struct Optimization: sắp xếp kích thước giảm dần)
struct DeviceOptimized {
    double voltage;    // 8 bytes
    float current;     // 4 bytes
    float power;       // 4 bytes
    int active_time;   // 4 bytes
    char status;       // 1 byte (có thể kèm padding cuối struct để chia hết cho alignment lớn nhất)
};

int main() {
    // In kích thước so sánh để thấy hiệu quả tối ưu bộ nhớ
    printf('Kich thuoc struct chua toi uu: %lu bytes\n', sizeof(struct DeviceUnoptimized));
    printf('Kich thuoc struct da toi uu: %lu bytes\n', sizeof(struct DeviceOptimized));
    printf('----------------------------------------\n');

    struct DeviceOptimized dev;
    double input_u, input_i, input_p;
    int input_t, input_status;

    // Nhập thông số thiết bị từ bàn phím
    printf('Nhap dien ap U (Volt): ');
    if (scanf('%lf', &input_u) != 1) { printf('Loi nhap lieu!\n'); return 1; }
    
    printf('Nhap dong dien I (Ampere): ');
    if (scanf('%lf', &input_i) != 1) { printf('Loi nhap lieu!\n'); return 1; }
    
    printf('Nhap cong suất P (Watt): ');
    if (scanf('%lf', &input_p) != 1) { printf('Loi nhap lieu!\n'); return 1; }

    printf('Nhap thoi gian hoat dong t (phut): ');
    if (scanf('%d', &input_t) != 1) { printf('Loi nhap lieu!\n'); return 1; }

    printf('Nhap trang thai status (0: Tat, 1: Bat): ');
    if (scanf('%d', &input_status) != 1) { printf('Loi nhap lieu!\n'); return 1; }

    // Kiểm tra dữ liệu rác / lỗi cảm biến
    if (input_u <= 0.0 || input_i < 0.0 || input_p < 0.0 || input_t < 0 || (input_status != 0 && input_status != 1)) {
        printf('LOI: Du lieu dau vao khong hop le hoac la du lieu rac tu cam bien!\n');
        return 1;
    }

    // Gán dữ liệu vào struct
    dev.voltage = input_u;
    dev.current = input_i;
    dev.power = input_p;
    dev.active_time = input_t;
    dev.status = (char)input_status;

    // Tính toán công suất thực tế P_calc = U * I
    double p_calc = dev.voltage * dev.current;

    // Kiểm tra sai lệch cảm biến công suất quá 5%
    double diff = dev.power - p_calc;
    if (diff < 0) diff = -diff;
    if (p_calc > 0 && (diff / p_calc) > 0.05) {
        printf('CANH BAO: Cam bien cong suất bao sai lech qua 5%% so voi U * I!\n');
    }

    // Kiểm tra quá tải dòng điện (I > 30.0A) hoặc công suất (P > 6600.0W)
    if (dev.current > 30.0 || dev.power > 6600.0) {
        printf('CANH BAO: QUA TAI NGUY HIEM! Dong dien hoac cong suất vuot nguong cho phép.\n');
    }

    // Kiểm tra rò rỉ điện năng (status == 0 nhưng I > 0.05A)
    if (dev.status == 0 && dev.current > 0.05) {
        printf('CANH BAO: RO RI DIEN HOẶC TAI AN! Thiet bi dang tat nhung van co dong dien ro.\n');
    }

    // Tính toán điện năng tiêu thụ kWh = (P * (t / 60.0)) / 1000.0 (đảm bảo chia số thực)
    double kwh = (dev.power * ((double)dev.active_time / 60.0)) / 1000.0;

    // In kết quả phân tích
    printf('----------------------------------------\n');
    printf('THONG TIN PHAN TICH THIẾT BI IOT:\n');
    printf('Dien ap U: %.2f V\n', dev.voltage);
    printf('Dong dien I: %.2f A\n', dev.current);
    printf('Cong suất do P: %.2f W (P_calc: %.2f W)\n', dev.power, p_calc);
    printf('Thoi gian hoat dong t: %d phut\n', dev.active_time);
    printf('Trang thai: %s\n', dev.status == 1 ? 'BAT' : 'TAT');
    printf('Dien nang tieu thu: %.4f kWh\n', kwh);

    return 0;
}
