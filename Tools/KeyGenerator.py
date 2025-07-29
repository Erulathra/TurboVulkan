

sdl_keys_file_path = '../cmake-build-debug/_deps/sdl-src/include/SDL3/SDL_keycode.h'

def main():
    sdl_key_names = []

    with open(sdl_keys_file_path) as sdl_keys_file:
        for line in sdl_keys_file:
            words = line.split()
            if len(words) > 1 and words[0] == '#define':
                if words[1][:5] == 'SDLK_':
                    sdl_key_names.append(words[1])

    corrected_names = []

    for key in sdl_key_names:
        key_name_no_prefix = key[5:]
        key_name_no_prefix = key_name_no_prefix.lower().capitalize()
        corrected_names.append(key_name_no_prefix)

    print(f'==========| KEY DEFINITIONS |==========')

    # generate keys
    for key in corrected_names:
        print(f'DECLARE_KEY({key}, false);')

    print(f'==========|     Switch       |==========')
    for i in range(len(sdl_key_names)):
        sdl_key = sdl_key_names[i]
        turbo_key = corrected_names[i]

        print(f'case {sdl_key}: return EKeys::{turbo_key};')

if __name__ == '__main__':
    main()