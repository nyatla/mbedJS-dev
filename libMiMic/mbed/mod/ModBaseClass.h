#pragma once
////////////////////////////////////////////////////////////////////////////////
// ModBaseClass.h
////////////////////////////////////////////////////////////////////////////////

#include "NyLPC_net.h"


namespace MiMic
{
    class HttpdConnection;

    class ModBaseClass
    {
    protected:
        char* _path;
    public:
        /**
         * @param i_path
         * target path
         * <pre>
         * ex.setParam("root")
         * </pre>
         */
        ModBaseClass(const char* i_path);
        ModBaseClass();
        virtual ~ModBaseClass();
    protected:
        /**
         * @param i_path
         * target path
         * <pre>
         * ex.setParam("")
         * </pre>
         */
        void setParam(const char* i_path);
        /**
         * URLとパスプレフィクスi_pathを比較して、処理対象のURLかを計算します。
         * URLに'/i_path/'を含むパスを処理対象とみなします。
         */
        virtual bool canHandle(HttpdConnection& i_connection);
    };
}