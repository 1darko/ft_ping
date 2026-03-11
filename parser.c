#include "ping.h"


int isnumeric(const char *str){
    while(*str){
        if(*str < '0' || *str > '9')
            return 0; 
        str++;
    }
    return 1;
}

void value_error(const char *value, char near_char, int type){
    fprintf(stderr, "ping: invalid value (`%s' near `%c')\n", type ? value + 2 : value, near_char);
    fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
}

int int_overflow(char *a)
{
    if(!a)
        return 0;
    if(strlen(a) > 10)
        return 1;
    if(strlen(a) == 10){
        if(strcmp(a, "2147483647") > 0)
            return 1;
    }
    return 0;
}
int flag_checker2(options *opts)
{
    if(opts->s_value < 0 || opts->s_value > 1473){
        fprintf(stderr, "ft_ping: invalid payload size '%d'\n", opts->s_value);
        return 1;
    }
    if(opts->w && (atoi(opts->w) <= 0 || int_overflow(opts->w))){
        fprintf(stderr, "ft_ping: invalid timeout '%s'\n", opts->w);
        return 1;
    }
    if(opts->flood && opts->i_is_set){
        fprintf(stderr, "ft_ping: options '-f' and '-i' are mutually exclusive\n");
        return 1;
    }
    if(int_overflow(opts->c)){
        fprintf(stderr, "ft_ping: invalid count '%s'\n", opts->c);
        return 1;
    }
    if(int_overflow(opts->i)){
        fprintf(stderr, "ft_ping: invalid interval '%s'\n", opts->i);
        return 1;
    }
    return 0;

}


static int parse_option_with_value(int *i, int ac, char **av, int j, char opt, char **dest)
{
    if (av[*i][j + 1] != '\0') {
        if (!isnumeric(&av[*i][j + 1]))
            return (value_error(av[*i], opt, 1), 1);
        *dest = &av[*i][j + 1];
    }
    else if (++(*i) < ac && av[*i][0] != '\0') {
        if (!isnumeric(av[*i]))
            return (value_error(av[*i], opt, 0), 1);
        *dest = av[*i];
    }
    else {
        fprintf(stderr, "ft_ping: option '-%c' requires an argument\n", opt);
        return 1;
    }
    return 0;
}

static int parse_flag(int *i, int ac, char **av, options *opts)
{
    int break_flag = 0;

    for (int j = 1; av[*i][j] != '\0' && !break_flag; j++) {
        switch (av[*i][j]) {
            case 'v': opts->v = 1; break;
            case '?': print_question_mark(); return 1;
            case 'D': opts->D = 1; break;
            case 'q': opts->q = 1; break;
            case 'r': opts->r = 1; break;
            case 'f': opts->flood = 1; opts->q = 1; break;
            case 'i':
                if (parse_option_with_value(i, ac, av, j, 'i', &opts->i))
                    return 1;
                opts->i_is_set = 1;
                break_flag = 1;
                break;
            case 'c':
                if (parse_option_with_value(i, ac, av, j, 'c', &opts->c))
                    return 1;
                break_flag = 1;
                break;
            case 's':
                if (parse_option_with_value(i, ac, av, j, 's', &opts->s))
                    return 1;
                break_flag = 1;
                break;
            case 'W':
                if (parse_option_with_value(i, ac, av, j, 'W', &opts->w))
                    return 1;
                break_flag = 1;
                break;
            default:
                fprintf(stderr, "ft_ping: invalid option -- '%c'\n", av[*i][j]);
                fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
                return 1;
        }
    }
    return 0;
}

int flag_checker(int ac, char **av, options *opts, const char **dst_ip)
{
    int ip_found = 0;
    // if bigger than 1473 or lesser than 0, should error out
    opts->s_value = PAYLOAD_SIZE;
    for (int i = 1; i < ac; i++) {
        if (av[i][0] == '-') {
            if (parse_flag(&i, ac, av, opts))
                return 1;
        }
        else if (!ip_found) {
            *dst_ip = av[i];
            ip_found = 1;
        }
        else {
            fprintf(stderr, "ft_ping: extra argument '%s'\n", av[i]);
            fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
            return 1;
        }
    }
    if (!ip_found) {
        fprintf(stderr, "ft_ping: destination address required\n");
        fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
        return 1;
    }
    if (flag_checker2(opts))
        return 1;
    opts->s_value = (opts->s ? atoi(opts->s) : PAYLOAD_SIZE);
    opts->c_value = (opts->c ? atoi(opts->c) : INT_MAX);
    return 0;
}


// int flag_checker(int ac, char **av, options *opts, const char **dst_ip)
// {
//     opts->s_value = PAYLOAD_SIZE; // to add option -s for payload size later
//     // if bigger than 1473 or lesser than 0, should error out
//     // -c to add
//     // -w to add
//     int break_flag;
//     int ip_found = 0;
//     for(int i = 1; i < ac; i++){
//         break_flag = 0;
//         if(av[i][0] == '-'){
//             for(int j = 1; av[i][j] != '\0' && !break_flag; j++){
//                 switch(av[i][j]){
//                     case 'v':
//                         opts->v = 1;
//                         break;  
//                     case '?':
//                         print_question_mark();
//                         return 1;
//                     case 'D':
//                         opts->D = 1;
//                         break;
//                     case 'q':
//                         opts->q = 1;
//                         break;
//                     case 'r':
//                         opts->r = 1;
//                         break;
//                     case 'f':
//                         opts->flood = 1;
//                         opts->q = 1;
//                         break;
//                     case 'i':
//                         if(av[i][j + 1] != '\0'){
//                             if(!isnumeric(&av[i][j + 1])){
//                                 return (value_error(av[i], 'i', 1), 1);
//                             }
//                             opts->i = &av[i][j + 1];
//                             opts->i_is_set = 1;
//                             break_flag = 1;
//                             break;
//                         }
//                         else if(++i < ac && av[i][0] != '\0') {
//                             if(!isnumeric(av[i])){
//                                 return (value_error(av[i], 'i', 0), 1);
//                             }
//                             opts->i = av[i];
//                             opts->i_is_set = 1;
//                         }
//                         else{
//                             fprintf(stderr, "ft_ping: option '-i' requires an argument\n");
//                             return 1;
//                         }
//                         break_flag = 1;
//                         break;
//                     case 'c':
//                         if(av[i][j + 1] != '\0'){
//                             if(!isnumeric(&av[i][j + 1])){
//                                 return (value_error(av[i], 'c', 1), 1);
//                             }
//                             opts->c = &av[i][j + 1];
//                         }
//                         else if(++i < ac && av[i][0] != '\0') {
//                             if(!isnumeric(av[i])){
//                                 return (value_error(av[i], 'c', 0), 1);
//                             }
//                             opts->c = av[i];
//                         }
//                         else{
//                             fprintf(stderr, "ft_ping: option '-c' requires an argument\n");
//                             return 1;
//                         }
//                         break_flag = 1;
//                         break;
//                     case 's':
//                         if(av[i][j + 1] != '\0'){
//                             if(!isnumeric(&av[i][j + 1])){
//                                 return (value_error(av[i], 's', 1), 1);
//                             }
//                             opts->s = &av[i][j + 1];
//                         }
//                         else if(++i < ac && av[i][0] != '\0') {
//                             if(!isnumeric(av[i])){
//                                 return (value_error(av[i], 's', 0), 1);
//                             }
//                             opts->s = av[i];
//                         }
//                         else{
//                             fprintf(stderr, "ft_ping: option '-s' requires an argument\n");
//                             return 1;
//                         }
//                         break_flag = 1;
//                         break;
//                     case 'W':
//                         if(av[i][j + 1] != '\0'){
//                             if(!isnumeric(&av[i][j + 1])){
//                                 return (value_error(av[i], 'W', 1), 1);
//                             }
//                             opts->w = &av[i][j + 1];
//                         }
//                         else if(++i < ac && av[i][0] != '\0') {
//                             if(!isnumeric(av[i])){
//                                 return (value_error(av[i], 'W', 0), 1);
//                             }
//                             opts->w = av[i];
//                         }
//                         else{
//                             fprintf(stderr, "ft_ping: option '-W' requires an argument\n");
//                             return 1;
//                         }
//                         break_flag = 1;
//                         break;
//                     default:
//                         fprintf(stderr, "ft_ping: invalid option -- '%c'\n", av[i][j]);
//                         fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
//                         return 1;
//                     }
//                 }
//             }
//             else if(ip_found == 0){
//                 *dst_ip = av[i];
//                 ip_found = 1;
//             }
//             else{
//                 fprintf(stderr, "ft_ping: extra argument '%s'\n", av[i]);
//                 fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
//                 return 1;
//             }
//         }
//         if(!ip_found){
//             fprintf(stderr, "ft_ping: destination address required\n");
//             fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
//             return 1;
//     }
//     if(flag_checker2(opts))
//         return 1;
//     opts->s_value = (opts->s ? atoi(opts->s) : PAYLOAD_SIZE);
//     opts->c_value = (opts->c ? atoi(opts->c) : INT_MAX);
//     return 0;
// };