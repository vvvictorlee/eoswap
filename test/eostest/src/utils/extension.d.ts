
interface Number {
    thousandsSeperator(): String;
    mul(b: number): number;

    div(b: number): number;

    divCeil(b: number): number;

    sub(b: number): number;

    add(b: number): number;

    sqrt(): number;
}
// interface Array<T> {
//     firstOrDefault(predicate: (item: T) => boolean): T;
//     where(predicate: (item: T) => boolean): T[];
//     orderBy(propertyExpression: (item: T) => any): T[];
//     orderByDescending(propertyExpression: (item: T) => any): T[];
//     orderByMany(propertyExpressions: [(item: T) => any]): T[];
//     orderByManyDescending(propertyExpressions: [(item: T) => any]): T[];
//     remove(item: T): boolean;
//     add(item: T): void;
//     addRange(items: T[]): void;
//     removeRange(items: T[]): void;
// }
// interface String {
//     isNullOrEmpty(this: string): boolean; format(...replacements: string[]): string;
// }

// export { }; 