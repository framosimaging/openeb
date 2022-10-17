/**********************************************************************************************************************
 * Copyright (c) Prophesee S.A.                                                                                       *
 *                                                                                                                    *
 * Licensed under the Apache License, Version 2.0 (the "License");                                                    *
 * you may not use this file except in compliance with the License.                                                   *
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0                                 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed   *
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.                      *
 * See the License for the specific language governing permissions and limitations under the License.                 *
 **********************************************************************************************************************/

// Data classes generated by psee_issd_convert.
// Parameters:
// issd_csv_format_version=1.0
// system=evk2_imx636_cd
// operator=Prophesee
// dry_run=False
// no_system=False
// strip_prefix=False
// sensor_version=None
// args=-s evk2_imx636_cd
// sensor=PseeEvk2IMX636
// date=2022-10-10T09:54:14
// psee_issd_version=1.2.0.1685+g1b82812af
// psee_sensor_lib_version=3.4.0.1480+g29c35578c
// issd_data_format_version=1.0

// clang-format off

#include <vector>

namespace Metavision {

const std::vector<RegisterOperation> issd_evk2_imx636_init = {
    RegisterOperation::Write(0x00000004, 0x00000111),
    RegisterOperation::Write(0x00000000, 0x00000089),
    RegisterOperation::Write(0x00000000, 0x0000008D),
    RegisterOperation::Write(0x00000000, 0x000000CD),
    RegisterOperation::Write(0x00000008, 0x0000004E),
    RegisterOperation::Write(0x00002004, 0x00002000),
    RegisterOperation::Write(0x00000014, 0x00000000),
    RegisterOperation::Write(0x00000600, 0x00000001),
    RegisterOperation::Write(0x00000044, 0x00000000),
    RegisterOperation::Write(0x00000048, 0x00030D41),
    RegisterOperation::Write(0x00000018, 0x00000000),
    RegisterOperation::Write(0x00700000, 0x00000001),
    RegisterOperation::Write(0x0070F000, 0x00400000),
    RegisterOperation::Write(0x0070F000, 0x00400000),
    RegisterOperation::Write(0x0070F000, 0x00400000),
    RegisterOperation::Write(0x0070F030, 0x00000000),
    RegisterOperation::Write(0x0070F030, 0x00000000),
    RegisterOperation::Write(0x0070F030, 0x00000000),
    RegisterOperation::Write(0x0070F030, 0x00000000),
    RegisterOperation::Write(0x0070F000, 0x00400000),
    RegisterOperation::Write(0x0070F030, 0x00000001),
    RegisterOperation::Write(0x0070F038, 0x01405002),
    RegisterOperation::Write(0x0070F038, 0x01405000),
    RegisterOperation::Write(0x0070F024, 0x00000000),
    RegisterOperation::Write(0x0070F024, 0x00000001),
    RegisterOperation::Delay(500000),
    RegisterOperation::Write(0x0070F024, 0x00000003),
    RegisterOperation::Delay(500000),
    RegisterOperation::Write(0x0070F030, 0x00000001),
    RegisterOperation::Write(0x0070F030, 0x00000001),
    RegisterOperation::Write(0x0070F038, 0x01405001),
    RegisterOperation::Delay(1000),
    RegisterOperation::Write(0x0070F030, 0x00000101),
    RegisterOperation::Delay(1000),
    RegisterOperation::Write(0x0070F030, 0x00000301),
    RegisterOperation::Write(0x0010B000, 0x00000158),
    RegisterOperation::Delay(300),
    RegisterOperation::Write(0x0010B044, 0x00000000),
    RegisterOperation::Write(0x0010B004, 0x0000000A),
    RegisterOperation::Write(0x0010B040, 0x00000000),
    RegisterOperation::Write(0x0010B0C8, 0x00000000),
    RegisterOperation::Write(0x0010B040, 0x00000000),
    RegisterOperation::Write(0x0010B040, 0x00000000),
    RegisterOperation::Write(0x00100000, 0x4F006442),
    RegisterOperation::Write(0x00100000, 0x0F006442),
    RegisterOperation::Write(0x001000B8, 0x00000400),
    RegisterOperation::Write(0x001000B8, 0x00000400),
    RegisterOperation::Write(0x0010B07C, 0x00000000),
    RegisterOperation::Write(0x0010B074, 0x00000002),
    RegisterOperation::Write(0x0010B078, 0x00000078),
    RegisterOperation::Write(0x001000C0, 0x00000108),
    RegisterOperation::Write(0x001000C0, 0x00000208),
    RegisterOperation::Write(0x0010B120, 0x00000001),
    RegisterOperation::Write(0x0010E120, 0x00000000),
    RegisterOperation::Write(0x0010B068, 0x00000004),
    RegisterOperation::Write(0x0010B07C, 0x00000001),
    RegisterOperation::Delay(10),
    RegisterOperation::Write(0x0010B07C, 0x00000003),
    RegisterOperation::Delay(1000),
    RegisterOperation::Write(0x001000B8, 0x00000401),
    RegisterOperation::Write(0x001000B8, 0x00000409),
    RegisterOperation::Write(0x00100000, 0x4F006442),
    RegisterOperation::Write(0x00100000, 0x4F00644A),
    RegisterOperation::Write(0x0010B080, 0x00000067),
    RegisterOperation::Write(0x0010B084, 0x0000000F),
    RegisterOperation::Write(0x0010B088, 0x00000027),
    RegisterOperation::Write(0x0010B08C, 0x00000027),
    RegisterOperation::Write(0x0010B090, 0x000000B7),
    RegisterOperation::Write(0x0010B094, 0x00000047),
    RegisterOperation::Write(0x0010B098, 0x0000002F),
    RegisterOperation::Write(0x0010B09C, 0x0000004F),
    RegisterOperation::Write(0x0010B0A0, 0x0000002F),
    RegisterOperation::Write(0x0010B0A4, 0x00000027),
    RegisterOperation::Write(0x0010B0AC, 0x00000028),
    RegisterOperation::Write(0x0010B0CC, 0x00000001),
    RegisterOperation::Write(0x0010A000, 0x000B0501),
    RegisterOperation::Delay(200),
    RegisterOperation::Write(0x0010A008, 0x00002405),
    RegisterOperation::Delay(200),
    RegisterOperation::Write(0x0010A004, 0x000B0501),
    RegisterOperation::Delay(200),
    RegisterOperation::Write(0x0010A020, 0x00000150),
    RegisterOperation::Delay(200),
    RegisterOperation::Write(0x0010B040, 0x00000007),
    RegisterOperation::Write(0x0010B064, 0x00000006),
    RegisterOperation::Write(0x0010B040, 0x0000000F),
    RegisterOperation::Delay(100),
    RegisterOperation::Write(0x0010B004, 0x0000008A),
    RegisterOperation::Delay(200),
    RegisterOperation::Write(0x0010B0C8, 0x00000003),
    RegisterOperation::Delay(200),
    RegisterOperation::Write(0x0010B044, 0x00000001),
    RegisterOperation::Write(0x0010B000, 0x00000159),
    RegisterOperation::Write(0x00107008, 0x00000001),
    RegisterOperation::Write(0x00107000, 0x00070001),
    RegisterOperation::Write(0x00108000, 0x0001E085),
    RegisterOperation::Write(0x00109008, 0x0000064A),
    RegisterOperation::Write(0x00100044, 0xCCFFCCCF),
    RegisterOperation::Write(0x00100004, 0xF0005042),
    RegisterOperation::Write(0x00100018, 0x00000200),
    RegisterOperation::Write(0x00101014, 0x11A1504D),
    RegisterOperation::Write(0x00109004, 0x00000000),
    RegisterOperation::Delay(1000),
    RegisterOperation::Write(0x00109000, 0x00000200)
};

const std::vector<RegisterOperation> issd_evk2_imx636_start = {
    RegisterOperation::Write(0x00000044, 0x00000001),
    RegisterOperation::Write(0x00000014, 0x00000001),
    RegisterOperation::Write(0x0070F000, 0x00400001),
    RegisterOperation::Write(0x0010B000, 0x00000159),
    RegisterOperation::Write(0x00109028, 0x00000000),
    RegisterOperation::Write(0x00109008, 0x0000064B),
    // Analog START
    RegisterOperation::Write(0x0010002C, 0x0022C724),
    RegisterOperation::Write(0x00100004, 0xF0005442),
    RegisterOperation::Write(0x00000018, 0x00000001),
    RegisterOperation::WriteField(0x00000008, 0x0000004F, 0x1)
};

const std::vector<RegisterOperation> issd_evk2_imx636_stop = {
    RegisterOperation::Write(0x00000018, 0x00000000),
    RegisterOperation::WriteField(0x00000008, 0x0000004E, 0x00000001),
    // Analog STOP
    RegisterOperation::Write(0x00100004, 0xF0005042),
    RegisterOperation::Write(0x0010002C, 0x0022C324),
    // Digital STOP
    RegisterOperation::Write(0x00109028, 0x00000002),
    RegisterOperation::Delay(1000),
    RegisterOperation::Write(0x00109008, 0x0000064A),
    RegisterOperation::Write(0x0010B000, 0x00000158),
    RegisterOperation::Delay(300)
};

const std::vector<RegisterOperation> issd_evk2_imx636_destroy = {
    // Analog DESTROY
    RegisterOperation::Write(0x00100070, 0x00400008),
    RegisterOperation::Write(0x0010006C, 0x0EE47114),
    RegisterOperation::Delay(500),
    RegisterOperation::Write(0x0010A00C, 0x00020400),
    RegisterOperation::Delay(500),
    RegisterOperation::Write(0x0010A010, 0x00008068),
    RegisterOperation::Delay(200),
    RegisterOperation::Write(0x00101104, 0x00000000),
    RegisterOperation::Delay(200),
    RegisterOperation::Write(0x0010A020, 0x00000050),
    RegisterOperation::Delay(200),
    RegisterOperation::Write(0x0010A004, 0x000B0500),
    RegisterOperation::Delay(200),
    RegisterOperation::Write(0x0010A008, 0x00002404),
    RegisterOperation::Delay(200),
    RegisterOperation::Write(0x0010A000, 0x000B0500),
    // Digital DESTROY
    RegisterOperation::Write(0x0010B044, 0x00000000),
    RegisterOperation::Write(0x0010B004, 0x0000000A),
    RegisterOperation::Write(0x0010B040, 0x0000000E),
    RegisterOperation::Write(0x0010B0C8, 0x00000000),
    RegisterOperation::Write(0x0010B040, 0x00000006),
    RegisterOperation::Write(0x0010B040, 0x00000004),
    RegisterOperation::Write(0x00100000, 0x4F006442),
    RegisterOperation::Write(0x00100000, 0x0F006442),
    RegisterOperation::Write(0x001000B8, 0x00000401),
    RegisterOperation::Write(0x001000B8, 0x00000400),
    RegisterOperation::Write(0x0010B07C, 0x00000000),
    RegisterOperation::Write(0x0070F030, 0x00000201),
    RegisterOperation::Write(0x0070F030, 0x00000001),
    RegisterOperation::Delay(1000),
    RegisterOperation::Write(0x0070F038, 0x01405000),
    RegisterOperation::Write(0x0070F024, 0x00000001),
    RegisterOperation::Write(0x0070F024, 0x00000000),
    RegisterOperation::Write(0x00000004, 0x00000777),
    RegisterOperation::Write(0x00000004, 0x00010111),
    RegisterOperation::Write(0x00000004, 0x00000000)
};

Issd issd_evk2_imx636_sequence = Issd{
    issd_evk2_imx636_init,
    issd_evk2_imx636_start,
    issd_evk2_imx636_stop,
    issd_evk2_imx636_destroy,
};

} // namespace Metavision

// clang-format on
