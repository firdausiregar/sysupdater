#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "models/package_model.h"

void test_model_initialization(void) {
    printf("[TEST] Testing Model Initialization...\n");
    AppModel *model = model_init();
    
    assert(model != NULL);
    assert(model->type >= PKG_MANAGER_APT && model->type <= PKG_MANAGER_UNKNOWN);
    assert(strlen(model->os_pretty_name) > 0);
    assert(strlen(model->user_cache_dir) > 0);
    
    printf("  [✓] Model initialized correctly. Detected OS: %s\n", model->os_pretty_name);
}

void test_translation_parser(void) {
    printf("[TEST] Testing PO Translation Parser...\n");
    AppModel *model = model_init();
    
    // Testing default msgid fallback
    const char *untranslated = model_translate_text(model, "NonExistentKey123");
    assert(strcmp(untranslated, "NonExistentKey123") == 0);

    // Testing translation insertion
    g_hash_table_insert(model->po_translations, g_strdup("Dashboard"), g_strdup("Panel Utama"));
    const char *translated = model_translate_text(model, "Dashboard");
    assert(strcmp(translated, "Panel Utama") == 0);
    
    printf("  [✓] Translation parser logic passed.\n");
}

void test_hold_session_storage(void) {
    printf("[TEST] Testing Hold Session Logic...\n");
    AppModel *model = model_init();
    
    // Test key insertion & retrieval
    g_hash_table_insert(model->hold_sessions, g_strdup("linux-image-generic"), GINT_TO_POINTER(1));
    assert(g_hash_table_lookup(model->hold_sessions, "linux-image-generic") != NULL);
    
    model_save_hold_session(model);
    
    // Clear and reload
    g_hash_table_remove_all(model->hold_sessions);
    model_load_hold_session(model);
    assert(g_hash_table_lookup(model->hold_sessions, "linux-image-generic") != NULL);
    
    printf("  [✓] Hold session persistence passed.\n");
}

int main(void) {
    printf("========================================\n");
    printf(" RUNNING SYSUPDATER PRO UNIT TESTS      \n");
    printf("========================================\n");
    
    test_model_initialization();
    test_translation_parser();
    test_hold_session_storage();
    
    printf("========================================\n");
    printf(" ALL UNIT TESTS PASSED SUCCESSFULLY!    \n");
    printf("========================================\n");
    return 0;
}
