# BÁO CÁO BÀI TẬP 03: PHÂN TÍCH TRADE-OFF STRUCT VÀ XỬ LÝ AN TOÀN ĐIỆN IOT GATEWAY

> 👤 **Học viên:** Hoàng Công Dũng | **Mã SV:** PTIT-HCM-009
> 🏫 **Môn học:** it102-rl-hoangcongdung

---

## 📊 Sơ đồ thiết kế hệ thống (Flowchart)

> 💡 *Sơ đồ dưới đây được render tự động trực tiếp trên GitHub bằng Mermaid. Bạn cũng có thể tải file **`bt3.drawio`** trong repository này để mở và chỉnh sửa trực tiếp trên [Draw.io (diagrams.net)](https://app.diagrams.net).* 

```mermaid
graph TD
  Start([Bắt đầu]) --> Input[Nhập U, I, t, status]
  Input --> CheckInput{Kiểm tra U <= 0, I < 0, t < 0, status != 0/1?}
  CheckInput -- Lỗi --> PrintErr[In thông báo lỗi dữ liệu rác] --> EndFail([Kết thúc: Dừng chương trình])
  CheckInput -- Hợp lệ --> Calc[Tính P_calc = U * I và kWh]
  Calc --> CheckDiff{Sai lệch (P - P_calc) > 5%?}
  CheckDiff -- Đúng --> WarnDiff[Cảnh báo: Cảm biến công suất báo sai lệch]
  CheckDiff -- Không --> CheckLoad{I > 30.0A hoặc P > 6600.0W?}
  WarnDiff --> CheckLoad
  CheckLoad -- Đúng --> WarnLoad[Cảnh báo: QUÁ TẢI NGUY HIỂM]
  CheckLoad -- Không --> CheckLeak{status == 0 và I > 0.05A?}
  WarnLoad --> CheckLeak
  CheckLeak -- Đúng --> WarnLeak[Cảnh báo: RÒ RỈ ĐIỆN HOẶC TẢI ẨN]
  CheckLeak -- Không --> PrintResult[In kết quả: Công suất, Điện năng kWh]
  WarnLeak --> PrintResult
  PrintResult --> End([Kết thúc: An toàn])
```

---

## 1. Phân tích Hiện trạng và Vấn đề Memory Padding trong C Struct

Trong ngôn ngữ C, trình biên dịch thực hiện cơ chế căn chỉnh bộ nhớ (Data Alignment) để CPU truy xuất dữ liệu hiệu quả theo kích thước từ (word size). Điều này khiến các trường dữ liệu có kích thước nhỏ (như char, int) bị chèn thêm các byte đệm (Padding Bytes) nếu nằm trước các trường có kích thước lớn hơn (như double, long long).

Đối với hệ thống IoT Gateway chạy trên vi điều khiển có dung lượng RAM rất hạn chế, việc bố trí thứ tự các thành phần trong struct một cách cảm tính gây lãng phí bộ nhớ nghiêm trọng, dẫn đến nguy cơ tràn bộ nhớ stack hoặc heap khi hệ thống mở rộng quản lý nhiều thiết bị.

- Struct chưa tối ưu (Flat Struct gốc): Các trường được khai báo ngẫu nhiên xen kẽ kiểu dữ liệu nhỏ và lớn, làm phát sinh nhiều byte đệm dư thừa.
- Hậu quả: Kích thước sizeof(struct) lớn hơn nhiều so với tổng dung lượng thực tế của các thành phần bên trong.

## 2. Đề xuất Hai Giải pháp Tái cấu trúc (Refactoring)

Để giải quyết triệt để vấn đề lãng phí bộ nhớ do Memory Padding, tôi đề xuất 2 phương án tái cấu trúc cấu trúc dữ liệu như sau:

Giải pháp 1: Sắp xếp lại thứ tự các thành phần trong struct phẳng (Flat Struct Optimization) theo thứ tự kích thước kiểu dữ liệu giảm dần (từ double, float xuống int và char). Cách này giúp gom các byte đệm xuống cuối struct hoặc loại bỏ hoàn toàn các khoảng trống trung gian.

Giải pháp 2: Sử dụng cấu trúc lồng nhau (Nested Struct) để gom nhóm các thông số cùng chức năng (ví dụ nhóm thông số điện năng riêng, nhóm trạng thái thiết bị riêng). Tuy nhiên, cách này có thể làm phát sinh thêm byte đệm ở ranh giới giữa các struct con nếu không sắp xếp khéo léo.

- Giải pháp 1 tối ưu tối đa về kích thước sizeof nhờ triệt tiêu khoảng trống giữa các thành phần.
- Giải pháp 2 tăng tính đóng gói và dễ quản lý mã nguồn khi số lượng thông số cảm biến tăng lên ở các phiên bản firmware sau.

## 3. Bảng so sánh chi tiết các giải pháp

Dưới đây là bảng phân tích trade-off giữa hai phương án tái cấu trúc để lựa chọn giải pháp tối ưu nhất cho vi điều khiển IoT Gateway:

| Tiêu chí so sánh | Giải pháp 1: Flat Struct Tối ưu thứ tự | Giải pháp 2: Nested Struct Lồng nhau |
| --- | --- | --- |
| Dung lượng RAM (sizeof) | Tối ưu nhỏ nhất, loại bỏ hầu hết padding bytes | Có thể lớn hơn do padding phát sinh ở các struct con |
| Độ phức tạp truy xuất | Đơn giản, truy xuất trực tiếp qua dấu chấm 'device.voltage' | Phức tạp hơn do phải qua nhiều lớp 'device.power.voltage' |
| Tính dễ bảo trì | Trung bình, các trường nằm phẳng hàng ngang | Cao, phân chia rõ ràng theo module chức năng |
| Khả năng mở rộng | Phù hợp với vi điều khiển tài nguyên thấp | Thích hợp cho hệ thống lớn trên vi xử lý mạnh |

## 4. Lựa chọn Giải pháp và Quy tắc Nghiệp vụ An toàn Tải điện

Dựa trên đặc thù của vi điều khiển IoT Gateway có dung lượng RAM hạn chế, Giải pháp 1 (Flat Struct Optimization theo thứ tự kích thước giảm dần) được lựa chọn là phương án tối ưu nhất vì giúp tiết kiệm từng byte bộ nhớ quý giá.

Chương trình C11 được thiết kế trong hàm main tuân thủ nghiêm ngặt các quy tắc nghiệp vụ: kiểm tra dữ liệu rác đầu vào (U <= 0, I < 0, t < 0, status khác 0/1), tính toán công suất thực tế P_calc = U * I, phát hiện sai lệch cảm biến quá 5%, cảnh báo quá tải dòng điện/công suất, phát hiện rò rỉ điện năng khi thiết bị tắt nhưng vẫn có dòng điện chạy qua, và tính toán điện năng tiêu thụ kWh chính xác trên số thực.

- Xử lý lỗi triệt để ngay từ bước nhập liệu từ bàn phím.
- Ép kiểu số thực cẩn thận trong phép tính kWh để tránh mất mát độ chính xác do chia số nguyên.

---

## 📁 Danh sách tệp tin nộp bài trong Repository
- 📝 `bt3.docx`: Báo cáo tài liệu phân tích nghiệp vụ hoàn chỉnh.
- 🎨 `bt3.drawio`: File thiết kế sơ đồ chuẩn theo quy định đề bài (mở trực tiếp bằng [Draw.io](https://app.diagrams.net) hoặc Lucidchart).
- 💻 `main.c`: Mã nguồn chương trình.
