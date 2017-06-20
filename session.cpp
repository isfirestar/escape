#include "session.h"
#include "os_util.hpp"
#include "toolkit.h"
#include "args.h"

#include <stdio.h>
#include <string>

static const char *getname(const char *path) {
    const char *symb = strrchr(path, '/');
    if (!symb) {
        symb = strrchr(path, '\\');
    }
    return symb;
}

///////////////////////////////////////////////////// session //////////////////////////////////////////////////////

session::session() : base_session(), client_init_finish(0) {
    tick = nsp::os::clock_gettime();
}

session::session(HTCPLINK lnk) : base_session(lnk), client_init_finish(0) {
    tick = nsp::os::clock_gettime();
}

session::~session() {
    if (file_task) {
        delete file_task;
    }
    if (escape_task) {
        delete escape_task;
    }
}

// ��Ӧ�ͻ��˵�½����
int session::on_login(const std::string &data) {
    struct proto_login login;
    int cb = data.size();
    if (login.build((const unsigned char *) data.c_str(), cb) < 0) {
        return -1;
    }
    
    // ��¼�����ӵĹ���ģʽ
    this->mode = login.mode;

    proto_login_ack login_ack;
    login_ack.err = 0;

    // ���� escape task ������
    if (CS_MODE_ESCAPE_TASK == mode) {
        try {
            // ���� escape task �ṹ
            if (!escape_task) {
                escape_task = new struct proto_escape_task;
                escape_task->head.type = PKTTYPE_ESCAPE_TASK_ACK;

                int size = getpkgsize();
                char *tmpstr = new char[size];
                escape_task->contex.assign(tmpstr, size);
                delete[]tmpstr;
            }
        } catch (...) {
            login_ack.err = ENOMEM;
        }
    }
    
    // CS_MODE_FILE_UPLOAD �� CS_MODE_FILE_DOWNLOAD ����Ҫ���κζ��⴦��
    // �ȴ� enable_filemode �����󵽴�
    else {
        nspinfo << "client file mode login."; 
    }

    return this->psend(&login_ack);
}

int session::on_login_client(const std::string &data) {
    proto_login_ack login_ack;
    int cb = data.size();
    if (login_ack.build((const unsigned char *) data.c_str(), cb) < 0) {
        return -1;
    }

    switch (this->mode) {
            // escape task ���͵����ӣ��������� ����ʼ����
        case CS_MODE_ESCAPE_TASK:
            {
                try {
                    escape_task = new struct proto_escape_task;
                    int size = getpkgsize();
                    char *tmpstr = new char[size];
                    escape_task->contex.assign(tmpstr, size);
                    delete[]tmpstr;
                } catch (...) {
                    return -1;
                }
                return send_escape_next();
            }
            break;

            // ��������ϴ��ļ��� ��Ӧ�������ļ��ڱ��غͷ���˵�Ȩ��
        case CS_MODE_FILE_UPLOAD:
            {
                // �����ϴ�����ǰ�� Ӧ�ó�ʼ�������ļ�Ȩ��
                try {
                    file_task = new file_mode(getfile(0), getpkgsize());
                    if (0 != file_task->open_it()) {
                        delete file_task;
                        file_task = nullptr;
                        client_inited = -1;
                        client_init_finish.sig();
                        return -1;
                    }
                } catch (...) {
                    return -1;
                }
                struct proto_enable_filemode filemode_request;
                int len;
                const char *file = getfile(&len);
                // �ϴ�����������ҪЯ��·��
                const char *symb = getname(file);
                std::string tmpstr;
                if (symb) {
                    tmpstr.assign(symb + 1, strlen(symb) - 1);
                }else{
                    tmpstr.assign(file);
                }
                filemode_request.path = tmpstr;
                filemode_request.block_size = getpkgsize();
                filemode_request.mode = ENABLE_FILEMODE_TEST; // �ȳ��Խ����ϴ��ļ����� ��Ҫ����˴����ļ��Ѿ����ڵ��쳣
                return psend(&filemode_request);
            }
            break;

            // �����ļ�������Ҫ���Ƿ���˵��ļ�Ȩ�ޣ� ����Ҫ���Ǳ��ض���ļ�Ȩ��
            // �������۱���Ȩ����Σ� ����Ҫ���� enable filemode ����
        case CS_MODE_FILE_DOWNLOAD:
            {
                struct proto_enable_filemode filemode_request;
            }
            break;

        default:
            return -1;
    }

    return 0;
}

// ����˴��� escape task
int session::on_escape_task(const std::string &data) {
    struct proto_escape_task escape_request;
    int cb = data.size();
    if (escape_request.build((const unsigned char *) data.c_str(), cb) < 0) {
        return -1;
    }

    // û�е�½ �� ����ģʽ������ ֱ�ӴٳɶϿ�
    if (!this->escape_task || this->mode != CS_MODE_ESCAPE_TASK) {
        return -1;
    }

    escape_task->head.id = escape_request.head.id;
    return this->psend(escape_task);
}

// �ͻ��˴��� escape task
int session::on_escape_task_client(const std::string &data) {
    struct proto_escape_task escape_response;
    int cb = data.size();
    if (escape_response.build((const unsigned char *) data.c_str(), cb) < 0) {
        return -1;
    }

    // ֪ͨ��ʼ�����
    if (client_inited < 0) {
        client_inited = 0;
        client_init_finish.sig();
    }
    
    return send_escape_next();
}

// ����˴����ļ�����
int session::on_enable_filemode(const std::string &data) {
    struct proto_enable_filemode filemode_enable_request;
    int cb = data.size();
    if (filemode_enable_request.build((const unsigned char *) data.c_str(), cb) < 0) {
        return -1;
    }

    // ����ģʽ�������ļ�ģʽ
    if (mode < CS_MODE_FILEMODE) {
        return -1;
    }

    // �ϴ��ļ�����, ���ݿͻ����������ͣ������ļ�Ȩ�ޣ� ���߽�������ѯ��
    if (mode == CS_MODE_FILE_UPLOAD) {
        struct proto_enable_filemode_ack filemode_enable_response;
        try {
            std::string file_path(nsp::os::get_module_directory<char>());
            file_path += POSIX__DIR_SYMBOL;
            file_path += filemode_enable_request.path;
            file_task = new file_mode(file_path, getpkgsize());

            if (filemode_enable_request.mode == ENABLE_FILEMODE_FORCE) {
                filemode_enable_response.err = file_task->cover_it();
            } else {
                filemode_enable_response.err = file_task->creat_it();
            }
            
            // ���۷���ʲô���� ������ EEXIST�� Ҳ�رձ��ε�������
            // EEXIST �������Ҫ�ͻ���ȷ���Ƿ񸲸Ǹ��ļ�
            if (0 != filemode_enable_response.err) {
                delete file_task;
                file_task = nullptr;
            }
        } catch (...) {
            filemode_enable_response.err = ENOMEM;
        }
        return psend(&filemode_enable_response);
    }

    return -1;
}

int session::on_enable_filemode_client(const std::string &data) {
    struct proto_enable_filemode_ack filemode_enable_ack;
    int cb = data.size();
    if (filemode_enable_ack.build((const unsigned char *) data.data(), cb) < 0) {
        return -1;
    }

    // ����ģʽ�������Ҫ��
    if (mode < CS_MODE_FILEMODE) {
        return -1;
    }

    // �ϴ�ģʽ
    if (mode == CS_MODE_FILE_UPLOAD) {
        
        // �ϴ�ģʽ�ߵ���һ���� �����ļ�Ȩ�޿϶��Ѿ���
        if (!file_task) {
            return -1;
        }

        // ��������û�и�·����Ȩ�ޣ� ����Ҫѯ���Ƿ񸲸Ǹ��ļ�
        if (filemode_enable_ack.err == EEXIST) {
            printf("file is existed,do you want to cover it?(yes/no):");
            char select_command[128];
            fgets(select_command, sizeof (select_command), stdin);
            if ('y' == select_command[0] || 'Y' == select_command[0]) {
                struct proto_enable_filemode filemode_request;
                int len;
                const char *path = getfile(&len);
                const char *symb = getname(path);
                std::string tmpstr;
                if (symb) {
                    tmpstr.assign(symb + 1, strlen(symb) - 1);
                }else{
                    tmpstr.assign(path);
                }
                //std::string tmpstr(symb + 1, strlen(symb) - 1);
                filemode_request.path = tmpstr;
                filemode_request.block_size = getpkgsize();
                filemode_request.mode = ENABLE_FILEMODE_FORCE; // �ȳ��Խ����ϴ��ļ����� ��Ҫ����˴����ļ��Ѿ����ڵ��쳣
                return psend(&filemode_request);
            }
            client_inited = -1;
            client_init_finish.sig();
            return -1;
        }
            
        // ������ �϶�û�з����κ����⣬ �ļ��Ѿ�����, ������һ���ϴ�����Ƭ
        else if (0 == filemode_enable_ack.err) {
            client_inited = send_upload_next();
            client_init_finish.sig();
            return 0;
        }
            
        // ���������쳣�� �������޷����� ֱ�Ӵٳɶ���
        else {
            client_inited = -1;
            client_init_finish.sig();
            return -1;
        }
    }

    return 0;
}

// ����˴����ϴ�����
int session::on_file_block(const std::string &data) {
    struct proto_file_block file_block;
    int cb = data.size();
    if (file_block.build((const unsigned char *) data.data(), cb) < 0) {
        return -1;
    }

    // �ļ�Ȩ�޺͹���ģʽ�����Ǻ�
    if (!file_task || this->mode < CS_MODE_FILEMODE) {
        return -1;
    }

    if (mode == CS_MODE_FILE_UPLOAD) {
        // ������ɣ� �Ͽ����Ӻ�ֱ�ӿ����ͷ��ļ�
        if (0 == file_block.data.size() || OFFSET_EOF == file_block.offset) {
            return -1;
        }

        // ����ʧ�ܶ��Զ�����Ϊ������
        auto wcb = file_block.data.size();
        if (wcb != (int)file_task->write_block(file_block.data.data(), file_block.offset, (int)wcb)) {
            return -1;
        }

        struct proto_file_block_ack file_block_ack;
        return psend(&file_block_ack);
    }
    return 0;
}

int session::on_file_block_client(const std::string &data) {
    struct proto_file_block_ack file_block;
    int cb = data.size();
    if (file_block.build((const unsigned char *) data.data(), cb) < 0) {
        return -1;
    }

    if (!file_task || this->mode < CS_MODE_FILEMODE ) {
        return -1;
    }

    // �������Է�����һƬ�ļ�
    if (this->mode == CS_MODE_FILE_UPLOAD) {
        return send_upload_next();
    }

    return 0;
}

void session::on_recvdata(const std::string &data) {
    int retval;
    struct proto_head head;
    int cb = data.size();
    if (head.build((const unsigned char *) data.c_str(), cb) < 0) {
        this->close();
        return;
    }

    retval = -1;
    switch (head.type.value_) {
        case PKTTYPE_LOGIN:
            retval = on_login(data);
            break;
        case PKTTYPE_LOGIN_ACK:
            retval = on_login_client(data);
            break;
        case PKTTYPE_ENABLE_FILEMODE:
            retval = on_enable_filemode(data);
            break;
        case PKTTYPE_ENABLE_FILEMODE_ACK:
            retval = on_enable_filemode_client(data);
            break;
        case PKTTYPE_FILE_BLOCK:
            retval = on_file_block(data);
            break;
        case PKTTYPE_FILE_BLOCK_ACK:
            retval = on_file_block_client(data);
            break;
        case PKTTYPE_ESCAPE_TASK:
            retval = on_escape_task(data);
            break;
        case PKTTYPE_ESCAPE_TASK_ACK:
            retval = on_escape_task_client(data);
            break;
        default:
            break;
    }

    if (retval < 0) {
        this->close();
    } else {
        // �ۻ������ֽ���
        my_stat.total_rx_ += data.size();
        my_stat.total_rx_ += nsp::proto::nspdef::protocol::Length();
        my_stat.sub_rx_ += data.size();
        my_stat.sub_rx_ += nsp::proto::nspdef::protocol::Length();
        // �ۻ�IO�����
        ++my_stat.io_counts_;
        ++my_stat.sub_io_counts_;
    }
}

extern void end_client();

void session::on_disconnected(const HTCPLINK previous) {
    // �ͻ������ӶϿ��� ֱ�������̣߳� ֪ͨ�˳�
    if (SESS_TYPE_CLIENT == type) {
        end_client();
    }
}

void session::print() {
    uint64_t current_tick = nsp::os::clock_gettime();
    uint64_t escaped_tick = current_tick - tick;
    uint32_t escaped_seconds = (uint32_t) (escaped_tick / 10000000);
    tick = current_tick;
    char total_tx_unit[16], total_rx_unit[16];

    // session �����ܹ���IO����
    uint64_t io_counts = my_stat.io_counts_;
    // session �����ܹ���Tx������
    posix__strcpy(total_tx_unit, cchof(total_tx_unit), "MB");
    double total_tx = (double) my_stat.total_tx_ / 1024 / 1024;
    if (total_tx > 1024) {
        total_tx /= 1024;
        posix__strcpy(total_tx_unit, cchof(total_tx_unit), "GB");
        if (total_tx > 1024) {
            total_tx /= 1024;
            posix__strcpy(total_tx_unit, cchof(total_tx_unit), "TB");
        }
    }
    // session �����ܹ���Rx������
    posix__strcpy(total_rx_unit, cchof(total_rx_unit), "MB");
    double total_rx = (double) my_stat.total_rx_ / 1024 / 1024;
    if (total_rx > 1024) {
        total_rx /= 1024;
        posix__strcpy(total_rx_unit, cchof(total_rx_unit), "GB");
        if (total_rx > 1024) {
            total_rx /= 1024;
            posix__strcpy(total_rx_unit, cchof(total_rx_unit), "TB");
        }
    }
    // ���ʱ�������ڵ�IOPS
    uint32_t iops = (uint32_t) my_stat.sub_io_counts_;
    if (escaped_seconds > 0) {
        iops /= escaped_seconds;
    }
    // ���ʱ�������ڵ�Tx�ٶ�
    char tx_speed[128];
    uint64_t tx_bps_u =  my_stat.sub_tx_;
    tx_bps_u *= 8;
    double tx_bps = (double) tx_bps_u;
    if (escaped_seconds > 0) {
        tx_bps /= escaped_seconds;
    }
    posix__sprintf(tx_speed, cchof(tx_speed), "%.1f bps", tx_bps);
    if (tx_bps / 1024 > 1) {
        posix__sprintf(tx_speed, cchof(tx_speed), "%.1f Kbps", tx_bps / 1024);
        tx_bps /= 1024;
    }
    if (tx_bps / 1024 > 1) {
        posix__sprintf(tx_speed, cchof(tx_speed), "%.1f Mbps", tx_bps / 1024);
        tx_bps /= 1024;
    }
    // ���ʱ�������ڵ�Rx�ٶ�
    char rx_speed[128];
    uint64_t rx_bps_u = my_stat.sub_rx_;
    rx_bps_u *= 8;
    double rx_bps = (double) rx_bps_u;
    if (escaped_seconds > 0) {
        rx_bps /= escaped_seconds;
    }
    posix__sprintf(rx_speed, cchof(rx_speed), "%.1f bps", rx_bps);
    if (rx_bps / 1024 > 1) {
        posix__sprintf(rx_speed, cchof(rx_speed), "%.1f Kbps", rx_bps / 1024);
        rx_bps /= 1024;
    }
    if (rx_bps / 1024 > 1) {
        posix__sprintf(rx_speed, cchof(rx_speed), "%.1f Mbps", rx_bps / 1024);
        rx_bps /= 1024;
    }

#if __x86_64__
    printf("\t%lu\t\t%.3f(%s)\t%.3f(%s)\t%u\t%s\t%s\n", io_counts, total_tx, total_tx_unit, total_rx, total_rx_unit, iops, tx_speed, rx_speed);
#else
    printf("\t%llu\t\t%.3f(%s)\t%.3f(%s)\t%u\t%s\t%s\n", io_counts, total_tx, total_tx_unit, total_rx, total_rx_unit, iops, tx_speed, rx_speed);
#endif


    // ��ɴ�ӡ����յ�ǰ����
    my_stat.sub_io_counts_ = 0;
    my_stat.sub_rx_ = 0;
    my_stat.sub_tx_ = 0;
}

// �����ͻ��ˣ� ��ʼ���ͻ��˵Ļ���������Ϣ�� �����������
int session::begin_client() {
    this->mode = getmode();
    this->type = gettype();
    struct proto_login login_request;
    login_request.mode = this->mode;
    return this->psend(&login_request);
}

int session::waitfor_init_finish() {
    if (0 != client_init_finish.wait(-1)) {
        return -1;
    }

    return client_inited;
}

// ������һƬ�ϴ�����
int session::send_upload_next() {
    struct proto_file_block file_block_upload;
    try {
        char *block = new char[file_task->get_blocksize()];
        int rdcb = file_task->read_block(block);
        if (rdcb <= 0) {
            file_block_upload.offset = OFFSET_EOF;
        } else {
            file_block_upload.offset = file_task->get_previous_offset();
            file_block_upload.data.assign(block, rdcb);
        }
        delete[]block;
    } catch (...) {
        return -1;
    }

    my_stat.sub_tx_ += file_block_upload.length();
    my_stat.total_tx_ += file_block_upload.length();
    return psend(&file_block_upload);
}

int session::send_escape_next(){
    // ����ͳ��
    my_stat.sub_tx_ += getpkgsize();
    my_stat.total_tx_ += getpkgsize();
    return psend(escape_task);
}