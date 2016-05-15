//**********************************************************************************
//EncryptPad Copyright 2016 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//This file is part of EncryptPad
//
//EncryptPad is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 2 of the License, or
//(at your option) any later version.
//
//EncryptPad is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with EncryptPad.  If not, see <http://www.gnu.org/licenses/>.
//**********************************************************************************
#include "file_helper.h"
#include <cstdio>
#include "win_file_reader.h"
#include "epad_utilities.h"

namespace EncryptPad
{
    FileHndl OpenInputLinux(const std::string &file_name)
    {
        if(file_name == "-")
            return FileHndl(stdin);

        FileHndl file(fopen(file_name.data(), "rb"));
        return file;
    }

    FileHndl OpenOutputLinux(const std::string &file_name)
    {
        if(file_name == "-")
            return FileHndl(stdout);

        FileHndl file(fopen(file_name.data(), "wb"));
        return file;
    }

    OpenFileResult OpenFile(const std::string &file_name, InPacketStreamFile &stm)
    {
#if defined(__MINGW__) || defined(__MINGW32__)
        FileHndl file = OpenInputWin(file_name);
#else
        FileHndl file = OpenInputLinux(file_name);
#endif

        if(!file)
            return OpenFileResult::Error;

        if(fseek(file.get(), 0, SEEK_END))
        {
            if(errno == ESPIPE || errno == EBADF || errno == EINVAL)
                return OpenFileResult::NotSeekable;

            return OpenFileResult::Error;
        }

        stream_length_type length = ftell(file.get());
        if(fseek(file.get(), 0, SEEK_SET))
        {
            return OpenFileResult::Error;
        }

        stm.Set(file, length);
        return OpenFileResult::OK;
    }

    OpenFileResult OpenFile(const std::string &file_name, OutPacketStreamFile &stm)
    {
#if defined(__MINGW__) || defined(__MINGW32__)
        FileHndl file = OpenOutputWin(file_name);
#else
        FileHndl file = OpenOutputLinux(file_name);
#endif
        if(!file)
            return OpenFileResult::Error;

        stm.Set(file);
        return OpenFileResult::OK;
    }

    bool RemoveFile(const std::string &file_name)
    {
#if defined(__MINGW__) || defined(__MINGW32__)
        return RemoveFileWin(file_name);
#else
        return remove(file_name.data()) == 0;
#endif
    }

    bool SaveToIOStream(int file_descriptor, const Botan::SecureVector<byte> &buffer)
    {
        FileHndl file = fdopen(file_descriptor, "w");
        if(!file.Valid())
            return false;

        size_t count = fwrite(buffer.begin(), 1, buffer.size(), file.get());
        return !ferror(file.get()) && count == buffer.size();
    }

    bool LoadFromIOStream(int file_descriptor, std::vector<byte> &buffer)
    {
        //TODO: increase the size after debugging
        const size_t kReadLength = 3;
        FileHndl file = fdopen(file_descriptor, "r");
        if(!file.Valid())
            return false;

        while(!feof(file.get()) && !ferror(file.get()))
        {
            size_t prev_size = buffer.size();
            buffer.resize(prev_size + kReadLength);
            size_t count = fread(buffer.data() + prev_size, 1, kReadLength, file.get());
            if(count != kReadLength)
                buffer.resize(prev_size + count);
        }

        return !ferror(file.get());
    }

    int GetStdinNo()
    {
        return fileno(stdin);
    }

    int GetStdoutNo()
    {
        return fileno(stdout);
    }
}
