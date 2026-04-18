#!/bin/bash

KERNEL_BIN="bin/unarddegos_ia32.o"
OUTPUT_FILE="src/symbols.c"

echo "// Auto-generated kernel symbol table" > $OUTPUT_FILE
echo "// Generated on: $(date)" >> $OUTPUT_FILE
echo "" >> $OUTPUT_FILE
echo "#include <module.h>" >> $OUTPUT_FILE
echo "#include <string.h>" >> $OUTPUT_FILE
echo "" >> $OUTPUT_FILE
echo "struct KernelSymbol" >> $OUTPUT_FILE
echo "{" >> $OUTPUT_FILE
echo "        const char *name;" >> $OUTPUT_FILE
echo "        void *addr;" >> $OUTPUT_FILE
echo "};" >> $OUTPUT_FILE
echo "" >> $OUTPUT_FILE
echo "static struct KernelSymbol KernelSymbols[] = {" >> $OUTPUT_FILE

KERNEL_BASE=$(nm -n $KERNEL_BIN 2>/dev/null | head -1 | awk '{print "0x"$1}')

nm -g --defined-only $KERNEL_BIN 2>/dev/null | \
    grep -v "\.text" | \
    awk '{print $1, $3, $2}' | \
    while read -r addr symbol type; do
        if [[ $symbol != "_"* ]] && \
           [[ $symbol != ".*"* ]] && \
           [[ $symbol != ".*"* ]] && \
           [[ $symbol != "end" ]] && \
           [[ $symbol != "__bss_start" ]]; then
            echo "        {\"$symbol\", (void*)0x$addr}," >> $OUTPUT_FILE
        fi
    done

echo "};" >> $OUTPUT_FILE
echo "" >> $OUTPUT_FILE
echo "const int KernelSymbolCount = sizeof(KernelSymbols) / sizeof(KernelSymbols[0]);" >> $OUTPUT_FILE
echo "" >> $OUTPUT_FILE
cat >> $OUTPUT_FILE << 'EOF'
void *FindKernelSymbol(const char *name)
{
        for (int i = 0; i < KernelSymbolCount; i++)
        {
                if (strncmp(KernelSymbols[i].name, name, 128) == 0)
                {
                        return KernelSymbols[i].addr;
                }
        }
        return NULL;
}
EOF

echo " [INFO] Generated $OUTPUT_FILE"
